#ifndef _DYN_NODE_H_
#define _DYN_NODE_H_

#include <stdbool.h>

#include "./DynWriteNode.h"

// Maximum number of nodes that can be in the huffman tree
#define MAX_NODES 10000

// Maximum height of the huffman tree
#define MAX_NODE_DEPTH 100

typedef struct DynNode DynNode;

typedef struct DynNode {
	DynNode *parent;
	DynNode *left;
	DynNode *right;

	int symbolLen;
	char symbol[255];

	int count;
	bool isRight;
} DynNode;

// Recursively frees this node's children, then itself
void destroyNode(DynNode *node);

// Recursively counts all nodes below this node (inc. this node)
int countNodes(const DynNode *node);

// Recursively tells all children that this node is its parent
void fixParents(DynNode *node);

bool findPathForSymbol(DynNode *nodePath, DynNode *node, int *pathLen, const char symbol[255], const int symbolLen);

void placeNodeInList(DynNode *node, DynWriteNode *nodeList, const int numNodes, int *curNodeIndex, int parent);

#endif
