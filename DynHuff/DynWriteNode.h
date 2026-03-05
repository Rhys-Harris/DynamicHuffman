#ifndef _DYN_WRITE_NODE_H_
#define _DYN_WRITE_NODE_H_

#include <stdbool.h>

#include "../def.h"

#define MAX_CHARS_PER_WRITE_NODE 264

typedef struct DynWriteNode {
	int parent;
	byte symbolLen;
	bool isRight;
	byte symbol[255];
} DynWriteNode;

#endif
