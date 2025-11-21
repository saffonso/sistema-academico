#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define T 3  // Grau mínimo da árvore B (ordem 5-6)
#define MAX_KEYS (2*T - 1)  // 5 chaves
#define MAX_CHILDREN (2*T)  // 6 filhos

// Tipo de chave suportado
typedef enum {
    KEY_INT,
    KEY_STRING
} KeyType;

// União para suportar diferentes tipos de chave
typedef struct {
    KeyType type;
    union {
        int int_key;
        char str_key[20];
    } value;
} Key;

// Estrutura de um nó da Árvore B
typedef struct BTreeNode {
    bool is_leaf;    
    int num_keys;              
    Key keys[MAX_KEYS];      
    long offsets[MAX_KEYS];    
    long children[MAX_CHILDREN];
    long offset_self;          
} BTreeNode;

// Estrutura de controle da Árvore B
typedef struct {
    FILE *idx_file;          
    long root_offset;         
    KeyType key_type;         
} BTree;

// Funções de comparação de chaves
int key_compare(Key *k1, Key *k2);
void key_copy(Key *dest, Key *src);
bool key_equals(Key *k1, Key *k2);

// Funções de criação de chaves
Key create_int_key(int value);
Key create_string_key(const char *value);

// Funções principais da Árvore B
BTree* btree_create(const char *idx_filename, KeyType key_type);
BTree* btree_open(const char *idx_filename);
void btree_close(BTree *tree);

// Operações da Árvore B
long btree_search(BTree *tree, Key key);
bool btree_insert(BTree *tree, Key key, long offset);
bool btree_delete(BTree *tree, Key key);

// Funções auxiliares de nó
BTreeNode* btree_node_create(bool is_leaf, KeyType key_type);
BTreeNode* btree_node_load(BTree *tree, long offset);
void btree_node_save(BTree *tree, BTreeNode *node);
void btree_node_free(BTreeNode *node);

// Funções auxiliares de inserção
void btree_split_child(BTree *tree, BTreeNode *parent, int index);
void btree_insert_nonfull(BTree *tree, BTreeNode *node, Key key, long offset);

// Funções auxiliares de busca
long btree_search_node(BTree *tree, BTreeNode *node, Key key);

// Funções auxiliares de remoção
void btree_merge(BTree *tree, BTreeNode *node, int index);
void btree_borrow_from_prev(BTree *tree, BTreeNode *node, int index);
void btree_borrow_from_next(BTree *tree, BTreeNode *node, int index);
void btree_fill(BTree *tree, BTreeNode *node, int index);
void btree_delete_from_node(BTree *tree, BTreeNode *node, Key key);


typedef struct {
    Key key;
    long offset;
} KeyOffsetPair;

typedef struct {
    KeyOffsetPair *pairs;
    int count;
    int capacity;
} KeyOffsetList;

KeyOffsetList* btree_get_all_pairs(BTree *tree);
void keyoffset_list_free(KeyOffsetList *list);

#endif // BTREE_H
