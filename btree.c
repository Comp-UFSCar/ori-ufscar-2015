/**
 * @file btree.c
 * @author João Vitor Brandão
 */

#include <stdbool.h>
#include "btree.h"
#include "stdlib.h"


btree_node *allocate_node(int order) {
    btree_node *node;
    node = (btree_node *) malloc(sizeof(btree_node));
    node->number_of_keys = 0; //number of active keys
    node->leaf = true;
    node->keys = (int *) malloc(2 * sizeof(int) * order - 1); //allocates an array of 2 x order - 1 positions
    node->children = (btree_node *) malloc(
            2 * sizeof(btree_node) * order); //allocates an array of 2 x order pointers to other nodes
    return node;
}

btree *btree_create(int order) {
    btree *tree = (btree *) malloc(sizeof(btree));
    tree->order = order;
    tree->root = allocate_node(order);
    return tree;
}

btree_node *btree_search(int key, btree_node *node) {
    int i = 0;

    while (i < node->number_of_keys && key > node->keys[i]) {
        i++;
    }

    if (i < node->number_of_keys && key == node->keys[i]) {
        return node;
    } else if (node->leaf == true) {
        return NULL;
    } else {
        return btree_search(key, &node->children[i]);
    }
}