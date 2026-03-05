#include <stdlib.h>

#include "DynNode.h"

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
