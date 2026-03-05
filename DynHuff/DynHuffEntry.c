#include <stddef.h>

#include "DynHuffEntry.h"

DynHuffEntry *searchForMatchingHuffEntry(DynHuffEntry *entries, const int numEntries, const char symbol[255], const int symbolLen) {
	for (int i = 0; i < numEntries; ++i) {
		DynHuffEntry *entry = entries+i;

		if (entry->symbolLen != symbolLen) {
			continue;
		}

		bool match = true;
		for (int j = 0; j < symbolLen; ++j) {
			if (entry->symbol[j] != symbol[j]) {
				match = false;
				break;
			}
		}

		if (match) {
			return entry;
		}
	}

	return NULL;
}
