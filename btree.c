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

btree_node *insert_nonfull(btree_node *node, int key, int order) {
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

btree_node *insert(btree *tree, int key) {
    btree_node *root = tree->root;
    //If root is full, allocates a new node
    if (root->number_of_keys == 2 * tree->order - 1) {
        btree_node *new_node = allocate_node(tree->order);
        tree->root = new_node;
        new_node->leaf = false;
        new_node->number_of_keys = 0;
        new_node->children[0] = root;
        split_child(new_node, 0, tree->order);
        return insert_nonfull(new_node, key, tree->order);
    } else {
        return insert_nonfull(root, key, tree->order);
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

btree_node *remove_key_from_node(btree_node *node, int key) {
    int i = 0;
    while (node->keys[i] != key && i < node->number_of_keys) {
        i++;
    }

    if (i == node->number_of_keys) {
        return NULL;
    }

    for (i; i < node->number_of_keys - 1; i++) {
        node->keys[i] = node->keys[i + 1];
    }

    node->number_of_keys--;

    return node;
}

btree_node *delete_key(btree *tree, btree_node *node, int key) {
    int i = 0;
    int index = 0;
    bool found = false;

    if (node == NULL) {
        return NULL;
    }
    //Try to find the key in the node. If it is not found, index is going to indicate root of the subtree that must contain the key
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
        //Case 3: if the key is not in a internal node, find the root of the subtree that must contain the key
    } else {
        //The key to be deleted is not in the tree
        if (node->leaf == true) {
            return NULL;
        }
        //Recursively calls deletion in the proper subtree
        if (node->children[index]->number_of_keys > tree->order - 1) {
            delete_key(tree, node->children[index], key);
        } else {
            //Parent of the root of the subtree that contains the key to be removed
            btree_node *parent = node;
            //Root of the subtree
            node = node->children[index];
            btree_node *left_sibling;
            btree_node *right_sibling;
            //Determines left and right siblings of the root of the subtree
            if (index == 0) {
                left_sibling = NULL;
                right_sibling = parent->children[1];
            } else if (index == parent->number_of_keys) {
                left_sibling = parent->children[parent->number_of_keys - 1];
                right_sibling = NULL;
            } else {
                left_sibling = parent->children[index - 1];
                right_sibling = parent->children[index + 1];
            }
            //Case 3.a.: If the root of the subtree has tree_order - 1 amount of keys but it has a sibling with at least the tree order amount of keys
            //Moves a key from the sibling node to the root of the subtree and moves a key from the parent of the root to the root of the subtree
            if (left_sibling != NULL && left_sibling->number_of_keys > tree->order - 1) {
                btree_node *right_subtree = left_sibling->children[left_sibling->number_of_keys];
                int temp = parent->keys[index - 1];
                parent->keys[index - 1] = left_sibling->keys[left_sibling->number_of_keys - 1];
                remove_key_from_node(left_sibling, parent->keys[index - 1]);
                int j = node->number_of_keys;
                for (j; j > 0; j--) {
                    node->keys[j] = node->keys[j - 1];
                    node->children[j + 1] = node->children[j];
                }
                node->keys[j] = temp;
                node->children[j + 1] = node->children[j];
                node->children[j] = right_subtree;
                node->number_of_keys++;
                delete_key(tree, node, key);
            } else if (right_sibling != NULL && right_sibling->number_of_keys > tree->order - 1) {
                btree_node *left_subtree = right_sibling->children[0];
                int temp = parent->keys[index];
                parent->keys[index] = right_sibling->keys[0];
                for (int j = 0; j < right_sibling->number_of_keys; j++) {
                    right_sibling->children[j] = right_sibling->children[j + 1];
                }
                remove_key_from_node(right_sibling, parent->keys[index]);
                node->keys[node->number_of_keys] = temp;
                node->children[node->number_of_keys + 1] = left_subtree;
                node->number_of_keys++;
                delete_key(tree, node, key);
                //Case 3.b: if both sibling nodes have tree_order -1 amount of keys, merge siblings
                //Merge with left_sibling
            } else if (left_sibling != NULL && left_sibling->number_of_keys == tree->order - 1) {
                //Copies the key from the parent to what is going to be the median position of the node
                left_sibling->keys[left_sibling->number_of_keys] = parent->keys[index - 1];
                left_sibling->number_of_keys++;
                //Copies the keys from the node to the left sibling
                int j = 0;
                for (j; j < node->number_of_keys; j++) {
                    left_sibling->keys[left_sibling->number_of_keys] = node->keys[j];
                    left_sibling->children[left_sibling->number_of_keys] = node->children[j];
                    left_sibling->number_of_keys++;
                }
                //Copies last children from node
                left_sibling->children[left_sibling->number_of_keys] = node->children[j];

                if (left_sibling->leaf) {
                    j = index;
                } else {
                    j = index - 1;
                    free(node);
                }

                for (j; j < parent->number_of_keys; j++) {
                    parent->children[j] = parent->children[j + 1];
                }
                remove_key_from_node(parent, parent->keys[index - 1]);
                if (parent == tree->root && parent->number_of_keys == 0) {
                    tree->root = left_sibling;
                }
                delete_key(tree, left_sibling, key);
                //Merge with right sibling
            } else if (right_sibling != NULL && right_sibling->number_of_keys == tree->order - 1) {
                //Copies the key from the parent to what is going to be the median position of the node
                node->keys[node->number_of_keys] = parent->keys[index];
                node->number_of_keys++;
                //Copies the keys from the right sibling to the node
                int j = 0;
                for (j; j < right_sibling->number_of_keys; j++) {
                    node->keys[node->number_of_keys] = right_sibling->keys[j];
                    node->children[node->number_of_keys] = right_sibling->children[j];
                    node->number_of_keys++;
                }
                //Copies last children from the right sibling
                node->children[node->number_of_keys] = right_sibling->children[j];

                if (right_sibling->leaf) {
                    j = index + 1;
                } else {
                    j = index;
                    free(right_sibling);
                }
                for (j; j < parent->number_of_keys; j++) {
                    parent->children[j] = parent->children[j + 1];
                }
                remove_key_from_node(parent, parent->keys[index]);
                if (parent == tree->root && parent->number_of_keys == 0) {
                    tree->root = node;
                }
                delete_key(tree, node, key);
            }
        }
    }
}
