#ifndef _DYN_WRITE_NODE_H_
#define _DYN_WRITE_NODE_H_

#include <stdbool.h>

#define MAX_CHARS_PER_WRITE_NODE 264

typedef struct DynWriteNode {
	int parent;
	unsigned char symbolLen;
	bool isRight;
	char symbol[255];
} DynWriteNode;

#endif
