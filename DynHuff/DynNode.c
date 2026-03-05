#include <stdlib.h>
#include <stdio.h>

#include "DynNode.h"

bool nodeFitsDesc(DynNode *node, const char symbol[255], const int symbolLen) {
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

bool findPathForSymbol(DynNode *nodePath, DynNode *node, int *pathLen, const char symbol[255], const int symbolLen) {
	if (*pathLen == MAX_NODE_DEPTH) {
		printf("Ran out of path space\n");
		return false;
	}

	// Add this node to the stack
	nodePath[*pathLen] = *node;
	++(*pathLen);

	// Check left
	if (node->left != NULL) {
		bool found = findPathForSymbol(nodePath, node->left, pathLen, symbol, symbolLen);
		if (found) {
			return true;
		}
	}

	// Check right
	if (node->right != NULL) {
		bool found = findPathForSymbol(nodePath, node->right, pathLen, symbol, symbolLen);
		if (found) {
			return true;
		}
	}

	// Check self
	if (nodeFitsDesc(node, symbol, symbolLen)) {
		return true;
	}

	// Pop off the stack
	--(*pathLen);
	
	// Return a fail
	return false;
}
