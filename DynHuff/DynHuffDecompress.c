#include <stdio.h>
#include <string.h>

#include "../BufView.h"

#include "DynHuffDecompress.h"

#define MAX_CHARS 100000

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
	printf("\n");

	return 0;
}
