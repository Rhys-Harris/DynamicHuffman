#ifndef _DYN_HUFF_ENTRY_H_
#define _DYN_HUFF_ENTRY_H_

#include <stdbool.h>

typedef struct DynHuffEntry {
		unsigned char symbolLen;
		char symbol[255];
		int count;
} DynHuffEntry;

DynHuffEntry *searchForMatchingHuffEntry(DynHuffEntry *entries, const int numEntries, const char symbol[255], const int symbolLen);

#endif
