#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "DynHuffEntry.h"

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

	// TODO: Have this work for more than just a pair
	printf("Finding consistent patterns\n");
	for (int i = 0; i < uniques; ++i) {
		DynHuffEntry entry = entries[i];

		char matcher = -1;
		bool doneFirstMatch = false;
		bool patternFound = false;

		// Check, is this symbol always followed by the same character?
		for (int j = 0; j < dataLen; ++j) {
			if (text[j] != entry.symbol[0]) {
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

		// TODO:Now, we double check this pattern

	}
	
	// Join together the patterns

	return entries;
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
	return 1;
}
