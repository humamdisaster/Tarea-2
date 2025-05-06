#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treemap.h"

typedef struct TreeNode TreeNode;


struct TreeNode {
    Pair* pair;
    TreeNode * left;
    TreeNode * right;
    TreeNode * parent;
};

struct TreeMap {
    TreeNode * root;
    TreeNode * current;
    int (*lower_than) (void* key1, void* key2);
};

int is_equal(TreeMap* tree, void* key1, void* key2){
    if(tree->lower_than(key1,key2)==0 &&  
        tree->lower_than(key2,key1)==0) return 1;
    else return 0;
}


TreeNode * createTreeNode(void* key, void * value) {
    TreeNode * new = (TreeNode *)malloc(sizeof(TreeNode));
    if (new == NULL) return NULL;
    new->pair = (Pair *)malloc(sizeof(Pair));
    new->pair->key = key;
    new->pair->value = value;
    new->parent = new->left = new->right = NULL;
    return new;
}

TreeMap * createTreeMap(int (*lower_than) (void* key1, void* key2)) 
{
    TreeMap * new = (TreeMap *)malloc(sizeof(TreeMap));
    if (new == NULL) return NULL;

    new->root = NULL;
    new->current =  NULL;
    new->lower_than = lower_than;
    //new->lower_than = lower_than;
    return new;
}


void insertTreeMap(TreeMap * tree, void* key, void * value) 
{
    TreeNode* current = tree->root;
    TreeNode* parent = NULL;

    while (current != NULL)
    {
        if (is_equal(tree, key, current->pair->key))
        {
            return;
        }
        parent = current;
        if (tree->lower_than(key, current->pair->key))
        {
            current = current->left;
        } else{
            current = current->right;
        }
    }

    TreeNode* newNode = createTreeNode(key, value);
    newNode->parent = parent;

    if (parent == NULL)
    {
        tree->root = newNode;
    } 
    else if (tree->lower_than(key, parent->pair->key))
    {
        parent->left = newNode;
    }
    else
    {
        parent->right = newNode;
    }

    tree->current = newNode;

}

TreeNode * minimum(TreeNode * x)
{
    if (x == NULL) return NULL;

    while(x->left != NULL)
    {
        x = x->left;
    }

    return x;
}


void removeNode(TreeMap * tree, TreeNode* node) 
{
    if (node == NULL) return;

    //sin hijos
    if (node->left == NULL && node->right == NULL)
    {
        if (node->parent == NULL)
        {
            tree->root = NULL;
        }
        else if (node->parent->left == node)
        {
            node->parent->left = NULL;
        }
        else
        {
            node->parent->right = NULL;
        }
        free(node->pair);
        free(node);
        return;
    }

    // un hijo
    if (node->left == NULL || node->right == NULL)
    {
        TreeNode* child = (node->left != NULL) ? node->left : node->right;

        if (node->parent == NULL)
        {
            tree->root = child;
        }
        else if (node->parent->left == node)
        {
            node->parent->left = child;
        }
        else 
        {
            node->parent->right = child;
        }

        child->parent = node->parent;
        free(node->pair);
        free(node);
        return;
    }

    //con tres hijos
    TreeNode* minNode = minimum(node->right);
    node->pair->key = minNode->pair->key;
    node->pair->value = minNode->pair->value;
    removeNode(tree, minNode);
}

void eraseTreeMap(TreeMap * tree, void* key){
    if (tree == NULL || tree->root == NULL) return;

    if (searchTreeMap(tree, key) == NULL) return;
    TreeNode* node = tree->current;
    removeNode(tree, node);
}




Pair * searchTreeMap(TreeMap * tree, void* key) 
{
    TreeNode * current = tree->root;

    while(current != NULL)
    {
        if (is_equal(tree, key, current->pair->key)){
            tree->current = current;
            return current->pair;
        } else if (tree->lower_than(key, current->pair->key)){
            current = current->left;
        } else{
            current = current->right;
        }
    }
    return NULL;
}


Pair * upperBound(TreeMap * tree, void* key) 
{
    if (tree == NULL || tree->root == NULL) return NULL;

    TreeNode* current = tree->root;
    TreeNode* ub_node = NULL;// puntero aux, que se usa para almacenar el nodo que podria ser el resultado

    while(current != NULL)
    {
        if (is_equal(tree, key, current->pair->key))
        {
            tree->current = current;
            return current->pair;
        }

        if (tree->lower_than(key, current->pair->key))
        {
            ub_node = current;
            current = current->left;
        }
        else current = current->right;
    }

    if (ub_node != NULL)
    {
        tree->current = ub_node;
        return ub_node->pair;
    }

    return NULL;
}

Pair * firstTreeMap(TreeMap * tree) 
{
    if (tree == NULL || tree->root == NULL) return NULL;
    
    TreeNode * min = minimum(tree->root);
    tree->current = min;

    if (min == NULL) return NULL;
    return min->pair;
}

Pair * nextTreeMap(TreeMap * tree) 
{
    if (tree == NULL || tree->current == NULL) return NULL;

    TreeNode * node = tree->current;

    //si tiene hijo derecho, el siguiente es el minimo del subarbol derecho
    if (node->right != NULL)
    {
        tree->current = minimum(node->right);
        return tree->current->pair;
    }

    //si no ai hijo derecho, se busca el primer ancestro para el cual,
    //este nodo esta en el subarbol izquierdo
    TreeNode * parent = node->parent;
    while(parent != NULL && parent->right == node)
    {
        node = parent;
        parent = parent->parent;
    }

    tree->current = parent;
    if (parent == NULL) return NULL;
    return parent->pair;
}
