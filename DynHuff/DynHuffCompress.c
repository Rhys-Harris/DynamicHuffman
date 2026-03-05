#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "DynHuffEntry.h"
#include "DynNode.h"

#include "DynHuffCompress.h"

#define MAX_CHARS 100000

// Returns the complete table, giving count for each char
DynHuffEntry *getUniqueSymbols(const char *text, const int dataLen, int *numSymbols) {
	// Used to count each char
	int staticTable[256];

	// Start all counts at 0
	memset(staticTable, 0, 256*sizeof(int));

	// Count each symbol
	for (int i = 0; text[i] != 0; ++i) {
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

		entries[tableIndex].symbol[0] = (char)i;
		entries[tableIndex].symbolLen = 1;
		entries[tableIndex].count = staticTable[i];
		++tableIndex;
	}

	return entries;
}

int mergeConsistentPatterns(DynHuffEntry *entries, int uniques, const char *text, const int dataLen) {
	// TODO: Have this work for more than just a pair
	printf("Finding consistent patterns\n");

	for (int i = 0; i < uniques; ++i) {
		DynHuffEntry *entry = entries+i;

		char matcher = -1;
		bool doneFirstMatch = false;
		bool patternFound = false;

		// Check, is this symbol always followed by the same character?
		for (int j = 0; j < dataLen; ++j) {
			if (text[j] != entry->symbol[0]) {
				continue;
			}

			if (!doneFirstMatch) {
				matcher = text[j+1];
				patternFound = true;
				doneFirstMatch = true;
				continue;
			}

			if (matcher != text[j+1]) {
				patternFound = false;
				break;
			}
		}

		// Couldn't find a consistent pattern
		if (!patternFound) {
			continue;
		}

		// Did we just try to match ourselves??
		if (matcher == entry->symbol[0]) {
			continue;
		}

		char matchFull[255];
		matchFull[0] = matcher;

		DynHuffEntry *other = searchForMatchingHuffEntry(entries, uniques, matchFull, 1);
		if (other == NULL) {
			// Shouldn't happen, but safer
			printf("ERR: Couldn't find entry\n");
			continue;
		}

		// Now, we double check this pattern
		// (e.g., does this other always occur with this preceding)
		if (other->count != entry->count) {
			continue;
		}

		// Merge results!
		entry->symbol[1] = other->symbol[0];
		entry->symbolLen++;

		// Delete other
		int otherIndex = (int)(
            (
				(long long)other -
				(long long)entries
			) /
			sizeof(DynHuffEntry)
		);
		--uniques;
		for (int j = otherIndex; j < uniques; ++j) {
			// Copy over each entry one back
			entries[j].symbolLen = entries[j+1].symbolLen;
			entries[j].count = entries[j+1].count;
			memcpy(entries[j].symbol, entries[j+1].symbol, entries[j].symbolLen);
		}

		// Were we moved back?
		if (otherIndex < i) {
			// Move back with it
			--i;
		}

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

errno_t dynHuffCompressFile(const char *infilename, const char *outfilename) {
	// Read in text
	FILE *f;
	if (fopen_s(&f, infilename, "rb")) {
		printf("Couldn't open input file\n");
		return 1;
	}

	char text[MAX_CHARS];
	memset(text, 0, MAX_CHARS);

	const int dataLen = (int)fread(text, sizeof(char), MAX_CHARS, f);

	fclose(f);

	return dynHuffCompress(text, outfilename, dataLen);
}

errno_t dynHuffCompress(const char *text, const char *outfilename, const int dataLen) {
	int numSymbols;
	DynHuffEntry *entries = getUniqueSymbols(text, dataLen, &numSymbols);
	if (entries == NULL) {
		printf("Couldn't get unique symbols\n");
		return 1;
	}

	numSymbols = mergeConsistentPatterns(entries, numSymbols, text, dataLen);

	sortEntries(entries, numSymbols);

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

	return 1;
}
