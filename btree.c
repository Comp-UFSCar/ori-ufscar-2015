/**
 * @file btree.c
 * @author João Vitor Brandão
 */

#include <stdbool.h>
#include <stdio.h>
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

void insert_nonfull(btree_node *node, int key, int order) {
    int i = node->number_of_keys;
    //If node is not a leaf, shift keys in order to open space to insert the new key
    if (node->leaf) {
        while (i >= 0 && key < node->keys[i - 1]) {
            node->keys[i] = node->keys[i - 1];
            i--;
        }
        node->keys[i] = key;
        node->number_of_keys++;
    } else {
        //If node is not a leaf, search for the child node where the key is potentially going to be inserted
        while (i > 0 && key < node->keys[i - 1]) {
            i--;
        }

//        i++;
        btree_node *child = node->children[i];
        //If child is full, split
        if (child->number_of_keys == 2 * order - 1) {
            split_child(node, i, order);
            if (key > node->keys[i]) {
                i++;
            }
        }
        //Recursively inserts key in the child node
        insert_nonfull(node->children[i], key, order);
    }
}

void insert(btree *tree, int key) {
    btree_node *root = tree->root;
    //If root is full, allocates a new node
    if (root->number_of_keys == 2 * tree->order - 1) {
        btree_node *new_node = allocate_node(tree->order);
        tree->root = new_node;
        new_node->leaf = false;
        new_node->number_of_keys = 0;
        new_node->children[0] = root;
        split_child(new_node, 0, tree->order);
        insert_nonfull(new_node, key, tree->order);
    } else {
        insert_nonfull(root, key, tree->order);
    }
}

void print_post_order(btree_node *root) {
    if (root != NULL) {
        for (int i = 0; i <= root->number_of_keys; i++) {
            if (!root->leaf) {
                print_post_order(root->children[i]);
            }
        }
        print_node(root);
    }
}

void print_node(btree_node *node) {
    for (int i = 0; i < node->number_of_keys; i++) {
        printf("%d ", node->keys[i]);
    }
    printf("\n");
}

int remove_key_from_node(btree_node *node, int key) {
    int i = 0;
    while (node->keys[i] != key && i < node->number_of_keys) {
        i++;
    }

    if (i == node->number_of_keys) {
        return 0;
    }

    for (i; i < node->number_of_keys - 1; i++) {
        node->keys[i] = node->keys[i + 1];
    }

    node->number_of_keys--;

    return 1;
}

int delete_key(btree *tree, btree_node *node, int key) {
    int i = 0;
    int index = 0;
    bool found = false;
    //Try to find the key in the node. If it is not found, index is going to indicate the path to go
    while (i < node->number_of_keys && key > node->keys[i]) {
        i++;
    }
    index = i;

    if (i < node->number_of_keys && key == node->keys[i]) {
        found = true;
    }

    if (found) {
        //Case 1: The node containing the key is found and is the leaf node
        //Allow deletion of key if the node has more than order - 1 keys or if the node is the root of the tree
        if (node->leaf && node->number_of_keys > tree->order - 1 || node->leaf && tree->root == node) {
            return remove_key_from_node(node, key);
        }

        //Case 2: The node containing the key is an internal node
        if (node->leaf == false) {
            //Case 2.a.: The left child node has a number of keys equal or greater than the order of the tree
            //In this case we swap the key to be deleted from the parent node with the last key of the left child and recursively delete it
            if (node->children[index]->number_of_keys > tree->order - 1) {
                btree_node *child = node->children[index];
                int temp = child->keys[child->number_of_keys - 1];
                child->keys[child->number_of_keys - 1] = node->keys[i];
                node->keys[i] = temp;
                delete_key(tree, child, key);
                //Case 2.b.: The right child node has a number of keys equal or greater than the order of the tree
                //In this case we swap the key to be deleted from the parent node with the first key of the right child and recursively delete it
            } else if (node->children[index + 1]->number_of_keys > tree->order - 1) {
                btree_node *child = node->children[index + 1];
                int temp = child->keys[0];
                child->keys[0] = node->keys[i];
                node->keys[i] = temp;
                delete_key(tree, child, key);
                //Case 2.c.: Both children have number of keys equal to the tree order-1
                //In this case we merge the children and move the key to be removed from the parent to the median position of the node containing
                //the keys merged from the children
            } else if (node->children[index]->number_of_keys == tree->order - 1 &&
                       node->children[index + 1]->number_of_keys == tree->order - 1) {
                btree_node *left_child = node->children[index];
                btree_node *right_child = node->children[index + 1];
                //Copies the key from the parent to what is going to be the median position of the node
                left_child->keys[left_child->number_of_keys] = node->keys[index];
                left_child->number_of_keys++;
                //Copies the keys from the right child to the left child
                for (int i = 0; i < right_child->number_of_keys; i++) {
                    left_child->keys[left_child->number_of_keys] = right_child->keys[i];
                    left_child->number_of_keys++;
                }
                for (int i = index + 1; i < node->number_of_keys; i++) {
                    node->children[i] = node->children[i + 1];
                }
                remove_key_from_node(node, key);
                free(right_child);
                delete_key(tree, left_child, key);
            }
        }
    }
}
