#ifndef _DYN_HUFF_ENTRY_H_
#define _DYN_HUFF_ENTRY_H_

#include <stdbool.h>

typedef struct DynHuffEntry {
		int symbolLen;
		char symbol[255];
		int count;
} DynHuffEntry;

#endif
