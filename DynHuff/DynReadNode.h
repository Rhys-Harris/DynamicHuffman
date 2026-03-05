#ifndef _DYN_READ_NODE_H_
#define _DYN_READ_NODE_H_

typedef struct DynReadNode {
	int left;
	int right;
	unsigned char symbolLen;
	char symbol[255];
} DynReadNode;

#endif
