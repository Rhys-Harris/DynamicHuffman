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
