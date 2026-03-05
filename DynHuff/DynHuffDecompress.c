#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../BufView.h"
#include "DynWriteNode.h"
#include "DynReadNode.h"

#include "DynHuffDecompress.h"

#define MAX_CHARS 500000

// Returns the index of the first char after the table
int readInNodeTable(DynWriteNode *rawTable, const byte *compText, const int numNodes) {
	printf("Reading in table\n");

	// Skip metadata
	int buffIndex = 13;

	for (int i = 0; i < numNodes; ++i) {
		DynWriteNode *node = rawTable+i;
		
		// 0 -> 3
		node->parent = readInt32FromBuff(buffIndex, (byte*)compText);
		buffIndex += 4;

		// 4
		node->symbolLen = compText[buffIndex];
		++buffIndex;

		// 5
		node->isRight = compText[buffIndex];
		++buffIndex;

		// 6 ->
		for (int j = 0; j < node->symbolLen; ++j, ++buffIndex) {
			node->symbol[j] = compText[buffIndex];
		}
	}

	return buffIndex;
}

DynReadNode *convertToReadableTable(const int numNodes, DynWriteNode *rawTable) {
	printf("Converting to readable table\n");
	DynReadNode *table = calloc(numNodes, sizeof(DynReadNode));
	if (table == NULL) {
		printf("Couldn't allocate table\n");
		return NULL;
	}

	printf("Converting...\n");

	for (int i = 0; i < numNodes; ++i) {
		DynWriteNode *wn = rawTable+i;

		memcpy(table[i].symbol, wn->symbol, wn->symbolLen);
		table[i].symbolLen = wn->symbolLen;
		if (wn->parent == -1) {
			continue;
		}

		if (wn->isRight) {
			table[wn->parent].right = i;
		} else {
			table[wn->parent].left = i;
		}
	}

	return table;
}

int decompress(const byte *inText, byte *out, int maxLength, DynReadNode *table, const int lastByteIndex, const int lastBitIndex) {
	int curByte = 0;
	int curBit = 0;

	DynReadNode *curNode = table+0;

	int outLen = 0;

	while (curByte < lastByteIndex || (curByte == lastByteIndex && curBit <= lastBitIndex)) {
		const byte bit = (byte)(inText[curByte]>>(7-curBit))&1;

		if (bit == 1) {
			curNode = table+curNode->right;
		} else {
			curNode = table+curNode->left;
		}

		// Hit a leaf!
		if (curNode->left == 0 && curNode->right == 0) {
			for (int i = 0; i < curNode->symbolLen; ++i) {
				if (outLen == maxLength) {
					printf("ERR: Ran out of output space!\n");
					return outLen;
				}
				out[outLen] = curNode->symbol[i];
				++outLen;
			}

			// Back to root
			curNode = table+0;
		}

		++curBit;
		if (curBit == 8) {
			curBit = 0;
			++curByte;
		}
	}

	return outLen;
}

errno_t dynHuffDecompressFile(const char *infilename, const char *outfilename) {
	// Read in text
	FILE *f;
	if (fopen_s(&f, infilename, "rb")) {
		printf("Couldn't open input file\n");
		return 1;
	}

	byte text[MAX_CHARS];
	memset(text, 0, MAX_CHARS);

	printf("Read %lli bytes\n", fread(text, sizeof(byte), MAX_CHARS, f));

	fclose(f);

	return dynHuffDecompress(text, outfilename);
}

errno_t dynHuffDecompress(const byte *compText, const char *outfilename) {
	// Get metadata
	printf("Getting metadata\n");
	const int numNodes = readInt32FromBuff(0, (const byte*)compText);
	const int dataLen = readInt32FromBuff(4, (const byte*)compText);
	const int lastByteIndex = readInt32FromBuff(8, (const byte*)compText);
	const int lastBitIndex = compText[12];

	printf("Num nodes %i\n", numNodes);
	printf("Original data length %i\n", dataLen);
	printf("Last byte index %i\n", lastByteIndex);
	printf("Last bit index %i\n", lastBitIndex);

	// Convert raw data to read nodes table
	printf("Allocating table\n");
	DynWriteNode *rawTable = calloc(numNodes, sizeof(DynWriteNode));
	if (rawTable == NULL) {
		printf("Couldn't allocate memory\n");
		return 1;
	}

	const int compBuffIndex = readInNodeTable(rawTable, compText, numNodes);

	// for (int i = 0; i < numNodes; ++i) {
	// 	DynWriteNode *n = rawTable+i;
	// 	if (n->symbolLen == 1) {
	// 		printf("NODE: '%c' L%i P%i R%i\n", n->symbol[0], n->symbolLen, n->parent, n->isRight);
	// 	} else {
	// 		printf("NODE: '%c%c' L%i P%i R%i\n", n->symbol[0], n->symbol[0], n->symbolLen, n->parent, n->isRight);
	// 	}
	// }

	DynReadNode *table = convertToReadableTable(numNodes, rawTable);
	if (table == NULL) {
		printf("Couldn't convert table\n");
		return 1;
	}

	// for (int i = 0; i < numNodes; ++i) {
	// 	DynReadNode *n = table+i;
	// 	if (n->symbolLen == 1) {
	// 		printf("NODE: '%c' L%i <%i >%i\n", n->symbol[0], n->symbolLen, n->left, n->right);
	// 	} else {
	// 		printf("NODE: '%c%c' L%i <%i >%i\n", n->symbol[0], n->symbol[0], n->symbolLen, n->left, n->right);
	// 	}
	// }

	printf("Destroying raw table\n");
	free(rawTable);

	printf("Isolating compressed text\n");
	const byte *inText = compText + compBuffIndex;

	printf("Creating output buffer\n");
	byte *out = calloc(dataLen, 1);

	printf("Decompressing...\n");
	int outLen = decompress(inText, out, dataLen, table, lastByteIndex, lastBitIndex);
	if (outLen != dataLen) {
		printf("Output size didn't match expected size\n");
		printf("\tExpected:\t%i\n", dataLen);
		printf("\tGot:\t\t%i\n", outLen);
		return 1;
	}

	printf("Destroying table\n");
	free(table);

	// Write output to file
	FILE *f;
	errno_t err = fopen_s(&f, outfilename, "w");
	if (err) {
		printf("Couldn't open output file\n");
		return 1;
	}
	fwrite(out, sizeof(byte), outLen, f);
	fclose(f);

	printf("Destroying output buffer\n");
	free(out);

	return 0;
}
