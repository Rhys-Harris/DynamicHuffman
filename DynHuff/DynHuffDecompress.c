#include <stdio.h>
#include <string.h>

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
	return 1;
}
