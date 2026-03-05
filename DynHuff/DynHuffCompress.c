#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <fileapi.h>

#include "DynHuffEntry.h"
#include "DynNode.h"
#include "CompStream.h"
#include "DynWriteNode.h"
#include "../BufView.h"
#include "../FRead.h"
#include "../FWrite.h"

#include "DynHuffCompress.h"

#define EXTRA_SIZE_BUFFER 5000

// Returns the complete table, giving count for each char
DynHuffEntry *getUniqueSymbols(const byte *text, const int dataLen, int *numSymbols) {
	// Used to count each char
	int staticTable[256];

	// Start all counts at 0
	memset(staticTable, 0, 256*sizeof(int));

	// Count each symbol
	for (int i = 0; i < dataLen; ++i) {
		staticTable[text[i]]++;
	}

	// Count how many uniques there are
	int uniques = 0;
	for (int i = 0; i < 256; ++i) {
		if (staticTable[i] != 0) {
			++uniques;
		}
	}
	*numSymbols = uniques;
	printf("Found %i unique symbols\n", uniques);

	// Create entry table
	DynHuffEntry *entries = malloc(uniques*sizeof(DynHuffEntry));
	if (entries == NULL) {
		printf("Couldn't allocate entry table");
		return NULL;
	}

	// Place counts into table
	int tableIndex = 0;
	for (int i = 0; i < 256; ++i) {
		if (staticTable[i] == 0) {
			continue;
		}

		entries[tableIndex].symbol[0] = (byte)i;
		entries[tableIndex].symbolLen = 1;
		entries[tableIndex].count = staticTable[i];
		++tableIndex;
	}

	return entries;
}

bool strMatchFoundAtPoint(const byte *strA, const byte *strB, const int len) {
	for (int i = 0; i < len; ++i) {
		if (strA[i] != strB[i]) {
			return false;
		}
	}
	return true;
}

bool charInString(const byte c, const byte *str, const int len) {
	for (int i = 0; i < len; ++i) {
		if (c == str[i]) {
			return true;
		}
	}
	return false;
}

int mergeConsistentPatterns(DynHuffEntry *entries, int uniques, const byte *text, const int dataLen) {
	// TODO: Deal with EOF
	printf("Finding consistent patterns\n");

	for (int i = 0; i < uniques; ++i) {
		DynHuffEntry *entry = entries+i;

		byte matcher = -1;
		bool doneFirstMatch = false;
		bool patternFound = false;

		// Check, is this symbol always followed by the same character?
		for (int j = 0; j < dataLen; ++j) {
			if (!strMatchFoundAtPoint(text+j, entry->symbol, entry->symbolLen)) {
				continue;
			}

			if (!doneFirstMatch) {
				matcher = text[j+entry->symbolLen];
				patternFound = true;
				doneFirstMatch = true;
				continue;
			}

			if (matcher != text[j+entry->symbolLen]) {
				patternFound = false;
				break;
			}
		}

		// Couldn't find a consistent pattern
		if (!patternFound) {
			continue;
		}

		// Did we just try to match ourselves??
		if (charInString(matcher, entry->symbol, entry->symbolLen)) {
			continue;
		}

		DynHuffEntry *other = searchForMatchingHuffEntry(entries, uniques, matcher);
		if (other == NULL) {
			// Shouldn't happen, but safer
			printf("ERR: Couldn't find entry\n");
			printf("'%c' (%i)\n", matcher, matcher);
			continue;
		}

		// Now, we double check this pattern
		// (e.g., does this other always occur with this preceding)
		if (other->count != entry->count) {
			continue;
		}

		// Merge results!
		for (int i = 0; i < other->symbolLen; ++i) {
			entry->symbol[entry->symbolLen] = other->symbol[i];
			entry->symbolLen++;
		}

		// Delete other
		int otherIndex = (int)(
			((long long)other - (long long)entries) /
			sizeof(DynHuffEntry)
		);
		--uniques;
		for (int j = otherIndex; j < uniques; ++j) {
			// Copy over each entry one back
			entries[j].symbolLen = entries[j+1].symbolLen;
			entries[j].count = entries[j+1].count;
			memcpy(entries[j].symbol, entries[j+1].symbol, entries[j].symbolLen);
		}

		// Try to match again
		--i;

		// printf("Successful merge!\n");
	}

	// TODO: Possible free old entry table, and alloc to new smaller mem

	return uniques;
}

void sortEntries(DynHuffEntry *entries, const int numSymbols) {
	for (int i = 0; i < numSymbols-1; ++i) {
		for (int j = 1; j < numSymbols-i; ++j) {
			if (entries[j-1].count < entries[j].count) {
				const DynHuffEntry temp = entries[j-1];
				entries[j-1] = entries[j];
				entries[j] = temp;
			}
		}
	}
}

errno_t createHuffmanTree(DynNode *nodes, const DynHuffEntry *entries, const int numSymbols, int *outNumNodes) {
	int numNodes = numSymbols;

	for (int i = 0; i < numSymbols; ++i) {
		nodes[i].parent = NULL;
		nodes[i].left = NULL;
		nodes[i].right = NULL;
		nodes[i].count = entries[i].count;
		nodes[i].isRight = false;
		nodes[i].symbolLen = entries[i].symbolLen;
		memcpy(nodes[i].symbol, entries[i].symbol, entries[i].symbolLen);
	}

	int i = numNodes-1;

	// While there are at least 2 root nodes
	printf("Creating huffman tree\n");
	while (i >= 1) {
		DynNode nodeA = nodes[i-1];
		DynNode nodeB = nodes[i];

		// Create a new parent node
		DynNode parent = (DynNode){NULL, NULL, NULL, nodeA.count + nodeB.count, false, 0};

		// Allocate space for each child node
		parent.left = malloc(sizeof(DynNode));
		if (parent.left == NULL) {
			printf("Couldn't allocate memory\n");
			return 1;
		}

		parent.right = malloc(sizeof(DynNode));
		if (parent.right == NULL) {
			printf("Couldn't allocate memory\n");
			return 1;
		}

		// Place in the children
		*parent.left = nodeA;
		*parent.right = nodeB;

		// Take the nodes out of the array
		numNodes -= 2;

		// Find index to place new parent
		int j = 0;
		while (j < numNodes) {
			if (nodes[j].count < parent.count) {
				break;
			}
			++j;
		}
		
		// Move everything over
		for (int k = numNodes-1; k >= j; --k) {
			nodes[k+1] = nodes[k];
		}

		// Place parent in
		nodes[j] = parent;
		
		// Increase length to include new node
		++numNodes;

		--i;
	}

	*outNumNodes = numNodes;

	return 0;
}

CompStream createCompressedText(const byte *text, const int dataLen, DynNode *root) {
	printf("Creating comp stream\n");

	const int MAX_NODE_DEPTH = nodeHeight(root);

	// Output scales with input size
	const int MAX_OUT_SIZE = EXTRA_SIZE_BUFFER + dataLen;

	CompStream out = EMPTY_COMP_STREAM;
	out.text = calloc(MAX_OUT_SIZE, sizeof(byte));
	if (out.text == NULL) {
		printf("Couldn't allocate memory\n");
		return out;
	}

	DynNode *nodePath = malloc(MAX_NODE_DEPTH*sizeof(DynNode));

	int pathLen = 0;

	int addition = 1;

	for (int i = 0; i < dataLen; i += addition) {
		bool found = findPathForSymbol(nodePath, root, &pathLen, text[i], MAX_NODE_DEPTH);

		if (!found) {
			printf("Couldn't find symbol '%c' ('%i')\n", text[i], text[i]);
			free(out.text);
			return EMPTY_COMP_STREAM;
		}

		for (int j = 1; j < pathLen; ++j) {
			byte byte = 0;
			DynNode curNode = nodePath[j];

			if (curNode.isRight) {
				byte = 1;
			}

			byte <<= (7-out.nextBitIndex);
			out.text[out.nextByteIndex] |= byte;

			++out.nextBitIndex;
			if (out.nextBitIndex == 8) {
				out.nextBitIndex = 0;
				out.nextByteIndex++;

				if (out.nextByteIndex == MAX_OUT_SIZE) {
					printf("Ran out of output space\n");
					return EMPTY_COMP_STREAM;
				}
			}
		}

		// Skip multiple chars if we just used a merger
		addition = nodePath[pathLen-1].symbolLen;

		pathLen = 0;
	}

	free(nodePath);

	// Calculate the length of the output string
	out.length = out.nextByteIndex + 1 - (out.nextBitIndex==0);

	return out;
}


DynWriteNode *createWriteTable(DynNode *root, const int numNodes) {
	DynWriteNode *nodeList = malloc(numNodes*sizeof(DynWriteNode));
	int curNodeIndex = 0;

	placeNodeInList(root, nodeList, numNodes, &curNodeIndex, -1);

	return nodeList;
}

int calcTableWriteSize(DynWriteNode *nodeList, const int numNodes) {
	int total = 0;

	for (int i = 0; i < numNodes; ++i) {
		total += 
			4 + // Parent's index
			1 + // Left or right node
			1 + // Symbol length
			nodeList[i].symbolLen; // Characters
	}

	return total;
}

int calcBytesNeededForWrite(DynWriteNode *nodeList, const int numNodes, CompStream stream) {
	const int bytesForMetadata = 
		4 + // Number of nodes
		4 + // Original file size
		4 + // Last byte index
		1;  // Last bit index

	const int bytesForTable = calcTableWriteSize(nodeList, numNodes);

	const int bytesForStream = stream.length;

	const int bytesNeeded = 
		bytesForMetadata +
		bytesForTable +
		bytesForStream;

	printf("Output bytes needed: %i\n", bytesNeeded);
	printf("\tMETAD: %i\n", bytesForMetadata);
	printf("\tTABLE: %i\n", bytesForTable);
	printf("\tCDATA: %i\n", bytesForStream);

	return bytesNeeded;
}

errno_t writeAllDataToBuffer(DynWriteNode *nodeList, const int numNodes, CompStream stream, const int dataLen, byte **outOut, int *outLen) {
	const int bytesNeeded = calcBytesNeededForWrite(nodeList, numNodes, stream);

	int lastByteIndex = stream.nextByteIndex;
	int lastBitIndex = stream.nextBitIndex-1;
	if (lastBitIndex == -1) {
		lastBitIndex = 7;
		--lastByteIndex;
	}

	// Confirm just in case
	if (lastByteIndex+1 != stream.length) {
		printf("Last byte index + 1 wasn't stream length\n");
		return 1;
	}

	printf("Allocating final string\n");
	byte *out = malloc(bytesNeeded);

	// Write metadata
	printf("Writing metadata\n");
	writeInt32ToBuff(numNodes, 0, (byte*)out);
	writeInt32ToBuff(dataLen, 4, (byte*)out);
	writeInt32ToBuff(lastByteIndex, 8, (byte*)out);
	out[12] = lastBitIndex;

	// Write table
	printf("Writing table\n");
	int buffIndex = 13;
	for (int i = 0; i < numNodes; ++i) {
		DynWriteNode *node = nodeList+i;
		
		// 0 -> 3
		writeInt32ToBuff(node->parent, buffIndex, (byte*)out);
		buffIndex += 4;

		// 4
		out[buffIndex] = (byte)node->symbolLen;
		++buffIndex;

		// 5
		out[buffIndex] = (byte)node->isRight;
		++buffIndex;

		// 6->
		for (int j = 0; j < node->symbolLen; ++j, ++buffIndex) {
			out[buffIndex] = node->symbol[j];
		}
	}

	// Write compressed text
	printf("Writing compressed text\n");
	for (int i = 0; i <= lastByteIndex; ++i, ++buffIndex) {
		out[buffIndex] = stream.text[i];
	}

	*outOut = out;
	*outLen = bytesNeeded;

	return 0;
}

errno_t dynHuffCompressFile(const char *infilename, const char *outfilename) {
	byte *text;
	long dataLen;
	errno_t err = FReadWhole(infilename, &text, &dataLen);
	err = dynHuffCompress(text, outfilename, dataLen);
	free(text);
	return err;
}

errno_t dynHuffCompress(const byte *text, const char *outfilename, const int dataLen) {
	int numSymbols;
	DynHuffEntry *entries = getUniqueSymbols(text, dataLen, &numSymbols);
	if (entries == NULL) {
		printf("Couldn't get unique symbols\n");
		return 1;
	}

	// printf("\nTABLE\n");
	// for (int i = 0; i < numSymbols; ++i) {
	// 	if (entries[i].symbolLen == 1) {
	// 		printf("%c %i %i\n", entries[i].symbol[0], entries[i].count, entries[i].symbolLen);
	// 	} else {
	// 		printf("%c%c %i %i\n", entries[i].symbol[0], entries[i].symbol[1], entries[i].count, entries[i].symbolLen);
	// 	}
	// }

	numSymbols = mergeConsistentPatterns(entries, numSymbols, text, dataLen);

	// printf("\nTABLE\n");
	// for (int i = 0; i < numSymbols; ++i) {
	// 	if (entries[i].symbolLen == 1) {
	// 		printf("%c %i %i\n", entries[i].symbol[0], entries[i].count, entries[i].symbolLen);
	// 	} else {
	// 		printf("%c%c %i %i\n", entries[i].symbol[0], entries[i].symbol[1], entries[i].count, entries[i].symbolLen);
	// 	}
	// }

	sortEntries(entries, numSymbols);

	// printf("\nTABLE\n");
	// for (int i = 0; i < numSymbols; ++i) {
	// 	if (entries[i].symbolLen == 1) {
	// 		printf("%c %i %i\n", entries[i].symbol[0], entries[i].count, entries[i].symbolLen);
	// 	} else {
	// 		printf("%c%c %i %i\n", entries[i].symbol[0], entries[i].symbol[1], entries[i].count, entries[i].symbolLen);
	// 	}
	// }

	DynNode *nodes = malloc(numSymbols*sizeof(DynNode));
	if (nodes == NULL) {
		printf("Couldn't allocate nodes for huffman tree\n");
		return 1;
	}
	int numNodes = 0;

	errno_t err = createHuffmanTree(nodes, entries, numSymbols, &numNodes);
	if (err) {
		printf("Couldn't create huffman tree\n");
		return 1;
	}

	printf("Destroying symbol counts\n");
	free(entries);

	// Pick root, allowing us to remove nodes array
	DynNode root = nodes[0];

	printf("Destroying nodes\n");
	free(nodes);

	// Get the number of nodes
	numNodes = countNodes(&root);
	printf("Tree made of %i nodes\n", numNodes);

	// Fix parents
	printf("Fixing parents\n");
	fixParents(&root);

	// Use the tree to compress the data
	CompStream stream = createCompressedText(text, dataLen, &root);
	if (stream.text == NULL) {
		printf("Couldn't compress text\n");
		return 1;
	}
	printf("Compressed Text Length: %i\n", stream.length);

	// Compress tree to a writable table
	DynWriteNode *nodeList = createWriteTable(&root, numNodes);

	// Destroy the huffman tree
	printf("Destroying huffman tree\n");
	destroyNode(&root);

	byte *out;
	int outLen;
	err = writeAllDataToBuffer(nodeList, numNodes, stream, dataLen, &out, &outLen);
	if (err) {
		printf("Couldn't write data to buffer\n");
		return 1;
	}

	// Destroy comp text buffer
	printf("Destroying comp text buffer\n");
	free(stream.text);

	// Destroy node list
	printf("Destroying write node list\n");
	free(nodeList);

	printf("Writing...\n");
	if (FWriteWhole(outfilename, out, outLen)) {
		printf("Couldn't write to file\n");
		return 1;
	}
	printf("Done write\n\n");

	// Destroy final buffer
	printf("Destroying output buffer\n");
	free(out);

	return 0;
}
