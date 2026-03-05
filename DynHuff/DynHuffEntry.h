#ifndef _DYN_HUFF_ENTRY_H_
#define _DYN_HUFF_ENTRY_H_

#include <stdbool.h>

#include "../def.h"

typedef struct DynHuffEntry {
		byte symbolLen;
		byte symbol[255];
		int count;
} DynHuffEntry;

DynHuffEntry *searchForMatchingHuffEntry(DynHuffEntry *entries, const int numEntries, const byte symbol);

#endif
