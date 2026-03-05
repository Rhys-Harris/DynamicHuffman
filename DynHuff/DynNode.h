#ifndef _DYN_NODE_H_
#define _DYN_NODE_H_

#include <stdbool.h>

#include "../def.h"
#include "./DynWriteNode.h"

// Maximum number of nodes that can be in the huffman tree
#define MAX_NODES 1000

typedef struct DynNode DynNode;

typedef struct DynNode {
	DynNode *parent;
	DynNode *left;
	DynNode *right;

	int count;
	bool isRight;

	int symbolLen;
	byte symbol[255];
} DynNode;

DynNode **findAllLeafNodes(DynNode *node, const int numLeafNodes);

int nodeHeight(const DynNode *node);

// Recursively frees this node's children, then itself
void destroyNode(DynNode *node);

// Recursively counts all nodes below this node (inc. this node)
int countNodes(const DynNode *node);

// Recursively tells all children that this node is its parent
void fixParents(DynNode *node);

void placeNodeInList(DynNode *node, DynWriteNode *nodeList, const int numNodes, int *curNodeIndex, int parent);

int findNodeStaringWithSymbol(DynNode **leafNodes, const int numLeafNodes, const byte symbol);

#endif
