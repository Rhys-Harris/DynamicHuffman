#include <stddef.h>

#include "DynHuffEntry.h"

DynHuffEntry *searchForMatchingHuffEntry(DynHuffEntry *entries, const int numEntries, const char symbol) {
	for (int i = 0; i < numEntries; ++i) {
		DynHuffEntry *entry = entries+i;
		if (entry->symbol[0] == symbol) {
			return entry;
		}
	}

	return NULL;
}
