/**
 * @file btree.h
 * @author João Vitor Brandão
 */

#ifndef ORI_UFSCAR_2015_BTREE_H
#define ORI_UFSCAR_2015_BTREE_H

/**
 * @brief A B-Tree Node
 *
 */
typedef struct btree_node {
    int number_of_keys; /**< Number keys currently stored in the node */
    bool leaf;
    /**< Indicates if the node is a leaf */
    int *keys;
    /**< Array containing the keys*/
    struct btree_node **children; /**< Pointers to the children nodes*/
} btree_node;

/**
 * @brief A B-Tree
 *
 * A B-Tree that has at most 2 x @p order - 1 keys in its nodes
 */
typedef struct btree {
    int order;
    /**< Order of the tree */
    btree_node *root;  /**<Root of the tree */
} btree;

/**
 * @brief Allocates a B-Tree node
 *
 * Allocates a B-Tree node that has at most 2 x @p order - 1 keys
 * @param order Order of the B-Tree
 * @return Returns a B-Tree node
 */
btree_node *allocate_node(int order);

/**
 * @brief Creates an empty B-Tree
 * @param order Order of the B-Tree
 * @return Returns an empty B-Tree
 */
btree *btree_create(int order);

/**
 * @brief Searches for a @p key in a tree with root in @p node
 * @param key The key to be searched
 * @param node Node that contains the root of the tree
 * @return If the key is found, returns the node containing the key
 */
btree_node *btree_search(int key, btree_node *node);

/**
 * @brief Splits a child node
 * @param node Parent node
 * @param child Position of the child node to be splitted
 * @param order Order of the B-Tree
 */
void split_child(btree_node *parent, int position, int order);

/**
 * @brief Inserts a key in a B-Tree
 * @param tree A B-Tree
 * @param key Key to be inserted
 */
void insert(btree *tree, int key);

#endif //ORI_UFSCAR_2015_BTREE_H
