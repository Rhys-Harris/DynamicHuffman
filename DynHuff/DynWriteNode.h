#ifndef _DYN_WRITE_NODE_H_
#define _DYN_WRITE_NODE_H_

#include <stdbool.h>

typedef struct DynWriteNode {
	int parent;
	unsigned char symbolLen;
	bool isRight;
	char symbol[255];
} DynWriteNode;

#endif
