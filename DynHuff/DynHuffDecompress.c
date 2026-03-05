#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../BufView.h"
#include "DynWriteNode.h"

#include "DynHuffDecompress.h"

#define MAX_CHARS 100000

void readInNodeTable(DynWriteNode *rawTable, const char *compText, const int numNodes) {
	printf("Reading in table\n");

	// Skip metadata
	int buffIndex = 13;

	for (int i = 0; i < numNodes; ++i) {
		DynWriteNode *node = rawTable+i;
		
		// 0 -> 3
		node->parent = readInt32FromBuff(buffIndex, (const unsigned char*)compText);
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
}

errno_t dynHuffDecompressFile(const char *infilename, const char *outfilename) {
	// Read in text
	FILE *f;
	if (fopen_s(&f, infilename, "rb")) {
		printf("Couldn't open input file\n");
		return 1;
	}

	char text[MAX_CHARS];
	memset(text, 0, MAX_CHARS);

	fread(text, sizeof(char), MAX_CHARS, f);

	fclose(f);

	return dynHuffDecompress(text, outfilename);
}

errno_t dynHuffDecompress(const char *compText, const char *outfilename) {
	// Get metadata
	printf("Getting metadata\n");
	const int numNodes = readInt32FromBuff(0, (const unsigned char*)compText);
	const int dataLen = readInt32FromBuff(4, (const unsigned char*)compText);
	const int lastByteIndex = readInt32FromBuff(8, (const unsigned char*)compText);
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

	readInNodeTable(rawTable, compText, numNodes);

	// Destroy write nodes
	printf("Destroying raw table\n");
	free(rawTable);

	return 0;
}
