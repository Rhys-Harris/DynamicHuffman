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
