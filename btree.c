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
    node->children = (btree_node **) malloc(
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
        return btree_search(key, node->children[i]);
    }
}

void split_child(btree_node *parent, int position, int order) {
    btree_node *child = parent->children[position];
    //Allocate a new node
    btree_node *new_node = allocate_node(order);
    new_node->leaf = child->leaf;
    new_node->number_of_keys = order - 1;

    //Copy half of the keys to the new node
    for (int i = 0; i < order - 1; i++) {
        new_node->keys[i] = child->keys[i + order];
    }
    //If the splitted node is not a leaf, adjust pointers to the child nodes
    if (!child->leaf) {
        for (int j = 0; j < order; j++) {
            new_node->children[j] = child->children[j + order];
        }
    }
    //Update number of keys of the node
    child->number_of_keys = order - 1;
    //Update pointers of the parent node
    for (int j = parent->number_of_keys; j > position + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    //Make the new node a child of the parent node
    parent->children[position + 1] = new_node;
    //Shifts the keys of the parent in order to open space to the key that is going to be moved from the child
    for (int j = parent->number_of_keys - 1; j > position - 1; j--) {
        parent->keys[j + 1] = parent->keys[j];
    }
    //Copy median key from the child to the parent
    parent->keys[position] = child->keys[order - 1];
    parent->number_of_keys++;
}