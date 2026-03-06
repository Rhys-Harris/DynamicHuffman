#include "../config.h"

#if TIME_DECOMP
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../FRead.h"
#include "../FWrite.h"
#include "../BufView.h"
#include "DynWriteNode.h"
#include "DynReadNode.h"

#include "DynHuffDecompress.h"

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
		// OPTIMIZE: Shouldn't need to do 7-curBit
		const byte bit = (byte)(inText[curByte]>>(7-curBit))&1;

		// OPTIMIZE: Can use memory tricks to index straight in
		if (bit == 1) {
			curNode = table+curNode->right;
		} else {
			curNode = table+curNode->left;
		}

		// Hit a leaf!
		// OPTIMIZE: Can AND both pointers then check against 0
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

		const byte atEndOfByte = curBit == 8;
		curByte += atEndOfByte; // If at end of byte, increment curByte
		curBit <<= (atEndOfByte<<3); // If at end of byte, shift all data to clear byte

		// if (curBit == 8) {
		// 	curBit = 0;
		// 	++curByte;
		// }
	}

	return outLen;
}

errno_t dynHuffDecompressFile(const char *infilename, const char *outfilename) {
	byte *text;
	long dataLen;
	errno_t err = FReadWhole(infilename, &text, &dataLen);
	if (err) {
		printf("Couldn't read input file\n");
		return err;
	}
	err = dynHuffDecompress(text, outfilename);
	free(text);
	return err;
}

errno_t dynHuffDecompress(const byte *compText, const char *outfilename) {
	#if TIME_DECOMP
	clock_t t1 = clock();
	#endif

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

	#if TIME_DECOMP
	clock_t t2 = clock();
	#endif

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

	#if TIME_DECOMP
	clock_t t3 = clock();
	#endif

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

	#if TIME_DECOMP
	clock_t t4 = clock();
	#endif

	printf("Decompressing...\n");
	int outLen = decompress(inText, out, dataLen, table, lastByteIndex, lastBitIndex);
	if (outLen != dataLen) {
		printf("Output size didn't match expected size\n");
		printf("\tExpected:\t%i\n", dataLen);
		printf("\tGot:\t\t%i\n", outLen);
		return 1;
	}

	#if TIME_DECOMP
	clock_t t5 = clock();
	#endif

	printf("Destroying table\n");
	free(table);

	printf("Writing...\n");
	if (FWriteWhole(outfilename, out, outLen)) {
		printf("Couldn't write to file\n");
		return 1;
	}
	printf("Done write\n\n");

	printf("Destroying output buffer\n");
	free(out);

	#if TIME_DECOMP
	printf("DECOMP TIME:\n");
	printf("\tREAD METADATA  :\t%lis (%lims)\n", (t2-t1)/CLOCKS_PER_SEC, t2-t1);
	printf("\tREAD NODE TABLE:\t%lis (%lims)\n", (t3-t2)/CLOCKS_PER_SEC, t3-t2);
	printf("\tCONVERT TABLE  :\t%lis (%lims)\n", (t4-t3)/CLOCKS_PER_SEC, t4-t3);
	printf("\tDECOMPRESS     :\t%lis (%lims)\n", (t5-t4)/CLOCKS_PER_SEC, t5-t4);
	#endif

	return 0;
}
