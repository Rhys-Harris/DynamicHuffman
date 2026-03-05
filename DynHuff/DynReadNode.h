#ifndef _DYN_READ_NODE_H_
#define _DYN_READ_NODE_H_

#include "../def.h"

typedef struct DynReadNode {
	int left;
	int right;
	byte symbolLen;
	byte symbol[255];
} DynReadNode;

#endif
