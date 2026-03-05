#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DynNode.h"

int nodeHeight(const DynNode *node) {
	int height = 0;

	if (node->left != NULL) {
		height = nodeHeight(node->left);
	}

	if (node->right != NULL) {
		height = __max(height, nodeHeight(node->right));
	}

	return height+1;
}

bool nodeFitsDesc(DynNode *node, const byte symbol[255], const int symbolLen) {
	if (node->symbolLen != symbolLen) {
		return false;
	}

	for (int i = 0; i < symbolLen; ++i) {
		if (node->symbol[i] != symbol[i]) {
			return false;
		}
	}

	return true;
}

void destroyNode(DynNode *node) {
	if (node->left != NULL) {
		destroyNode(node->left);
		free(node->left);
	}

	if (node->right != NULL) {
		destroyNode(node->right);
		free(node->right);
	}
}

int countNodes(const DynNode *node) {
	int count = 1;

	if (node->left != NULL) {
		count += countNodes(node->left);
	}

	if (node->right != NULL) {
		count += countNodes(node->right);
	}

	return count;
}

void fixParents(DynNode *node) {
	if (node->left != NULL) {
		fixParents(node->left);
		node->left->parent = node;
		node->left->isRight = false;
	}

	if (node->right != NULL) {
		fixParents(node->right);
		node->right->parent = node;
		node->left->isRight = true;
	}
}

int findNodeStaringWithSymbol(DynNode **leafNodes, const int numLeafNodes, const byte symbol) {
	for (int i = 0; i < numLeafNodes; ++i) {
		DynNode *leafNode = leafNodes[i];

		if (leafNode->symbol[0] == symbol) {
			return i;
		}
	}

	return -1;
}

void placeNodeInList(DynNode *node, DynWriteNode *nodeList, const int maxNodes, int *curNodeIndex, int parent) {
	if (*curNodeIndex == maxNodes) {
		printf("Ran out of space to add nodes\n");
		return;
	}

	// Place this node in the list
	nodeList[*curNodeIndex] = (DynWriteNode){
		parent,
		node->symbolLen,
		node->isRight,
		// Symbol added below
	};
	memcpy(nodeList[*curNodeIndex].symbol, node->symbol, node->symbolLen);

	const int thisIndex = *curNodeIndex;

	if (node->left != NULL) {
		// Go to the next node in list
		(*curNodeIndex)++;

		// Place left side in
		placeNodeInList(node->left, nodeList, maxNodes, curNodeIndex, thisIndex);
	}

	if (node->right != NULL) {
		// Go to the next node in list
		(*curNodeIndex)++;

		// Place right side in
		placeNodeInList(node->right, nodeList, maxNodes, curNodeIndex, thisIndex);
	}
}

void recFindAllLeafNodes(DynNode *node, DynNode **leafNodes, int *curLeafNode) {
	bool leaf = true;

	if (node->left != NULL) {
		leaf = false;
		recFindAllLeafNodes(node->left, leafNodes, curLeafNode);
	}

	if (node->right != NULL) {
		leaf = false;
		recFindAllLeafNodes(node->right, leafNodes, curLeafNode);
	}

	if (!leaf) {
		return;
	}

	leafNodes[*curLeafNode] = node;
	++(*curLeafNode);
}

DynNode **findAllLeafNodes(DynNode *node, const int numLeafNodes) {
	DynNode **leafNodes = malloc(numLeafNodes*sizeof(DynNode*));
	if (leafNodes == NULL) {
		printf("Couldn't allocate leaf nodes\n");
		return NULL;
	}

	int curLeafNode = 0;

	recFindAllLeafNodes(node, leafNodes, &curLeafNode);

	return leafNodes;
}
