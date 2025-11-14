#include "btree.h"

// ==================== FUNÇÕES DE CHAVE ====================

// Compara duas chaves: retorna <0 se k1<k2, 0 se k1==k2, >0 se k1>k2
int key_compare(Key *k1, Key *k2) {
    if (k1->type != k2->type) {
        fprintf(stderr, "Erro: tipos de chave incompatíveis\n");
        exit(1);
    }

    if (k1->type == KEY_INT) {
        return k1->value.int_key - k2->value.int_key;
    } else {
        return strcmp(k1->value.str_key, k2->value.str_key);
    }
}

// Copia uma chave
void key_copy(Key *dest, Key *src) {
    dest->type = src->type;
    if (src->type == KEY_INT) {
        dest->value.int_key = src->value.int_key;
    } else {
        strcpy(dest->value.str_key, src->value.str_key);
    }
}

// Verifica se duas chaves são iguais
bool key_equals(Key *k1, Key *k2) {
    return key_compare(k1, k2) == 0;
}

// Cria uma chave inteira
Key create_int_key(int value) {
    Key key;
    key.type = KEY_INT;
    key.value.int_key = value;
    return key;
}

// Cria uma chave string
Key create_string_key(const char *value) {
    Key key;
    key.type = KEY_STRING;
    strncpy(key.value.str_key, value, 19);
    key.value.str_key[19] = '\0';
    return key;
}

// ==================== FUNÇÕES DE NÓ ====================

// Cria um novo nó
BTreeNode* btree_node_create(bool is_leaf, KeyType key_type) {
    BTreeNode *node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) {
        fprintf(stderr, "Erro: falha ao alocar memória para nó\n");
        exit(1);
    }

    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->offset_self = -1;

    // Inicializa arrays
    for (int i = 0; i < MAX_KEYS; i++) {
        node->keys[i].type = key_type;
        node->offsets[i] = -1;
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = -1;
    }

    return node;
}

// Carrega um nó do arquivo
BTreeNode* btree_node_load(BTree *tree, long offset) {
    if (offset < 0) return NULL;

    BTreeNode *node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) {
        fprintf(stderr, "Erro: falha ao alocar memória para nó\n");
        exit(1);
    }

    fseek(tree->idx_file, offset, SEEK_SET);
    fread(node, sizeof(BTreeNode), 1, tree->idx_file);

    return node;
}

// Salva um nó no arquivo
void btree_node_save(BTree *tree, BTreeNode *node) {
    if (node->offset_self < 0) {
        // Novo nó: adiciona no final do arquivo
        fseek(tree->idx_file, 0, SEEK_END);
        node->offset_self = ftell(tree->idx_file);
    } else {
        // Nó existente: sobrescreve
        fseek(tree->idx_file, node->offset_self, SEEK_SET);
    }

    fwrite(node, sizeof(BTreeNode), 1, tree->idx_file);
    fflush(tree->idx_file);
}

// Libera memória de um nó
void btree_node_free(BTreeNode *node) {
    free(node);
}

// ==================== FUNÇÕES DE ÁRVORE ====================

// Cria uma nova árvore B
BTree* btree_create(const char *idx_filename, KeyType key_type) {
    BTree *tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) {
        fprintf(stderr, "Erro: falha ao alocar memória para árvore\n");
        exit(1);
    }

    tree->idx_file = fopen(idx_filename, "w+b");
    if (!tree->idx_file) {
        fprintf(stderr, "Erro: não foi possível criar arquivo %s\n", idx_filename);
        free(tree);
        exit(1);
    }

    tree->key_type = key_type;

    // Reserva espaço para metadados no início do arquivo
    long placeholder = 0;
    fseek(tree->idx_file, 0, SEEK_SET);
    fwrite(&placeholder, sizeof(long), 1, tree->idx_file);
    fwrite(&key_type, sizeof(KeyType), 1, tree->idx_file);
    fflush(tree->idx_file);

    // Cria nó raiz vazio
    BTreeNode *root = btree_node_create(true, key_type);
    btree_node_save(tree, root);
    tree->root_offset = root->offset_self;

    // Atualiza offset da raiz no início do arquivo
    fseek(tree->idx_file, 0, SEEK_SET);
    fwrite(&tree->root_offset, sizeof(long), 1, tree->idx_file);
    fflush(tree->idx_file);

    btree_node_free(root);

    return tree;
}

// Abre uma árvore B existente
BTree* btree_open(const char *idx_filename) {
    BTree *tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) {
        fprintf(stderr, "Erro: falha ao alocar memória para árvore\n");
        exit(1);
    }

    tree->idx_file = fopen(idx_filename, "r+b");
    if (!tree->idx_file) {
        // Arquivo não existe, retorna NULL
        free(tree);
        return NULL;
    }

    // Lê offset da raiz e tipo de chave do início do arquivo
    fseek(tree->idx_file, 0, SEEK_SET);
    fread(&tree->root_offset, sizeof(long), 1, tree->idx_file);
    fread(&tree->key_type, sizeof(KeyType), 1, tree->idx_file);

    return tree;
}

// Fecha uma árvore B
void btree_close(BTree *tree) {
    if (tree) {
        if (tree->idx_file) {
            fclose(tree->idx_file);
        }
        free(tree);
    }
}

// ==================== BUSCA ====================

// Busca uma chave no nó (recursivo)
long btree_search_node(BTree *tree, BTreeNode *node, Key key) {
    int i = 0;

    // Encontra a primeira chave >= key
    while (i < node->num_keys && key_compare(&key, &node->keys[i]) > 0) {
        i++;
    }

    // Chave encontrada
    if (i < node->num_keys && key_equals(&node->keys[i], &key)) {
        long offset = node->offsets[i];
        return offset;
    }

    // Chave não encontrada e é folha
    if (node->is_leaf) {
        return -1;
    }

    // Busca no filho apropriado
    BTreeNode *child = btree_node_load(tree, node->children[i]);
    long result = btree_search_node(tree, child, key);
    btree_node_free(child);

    return result;
}

// Busca uma chave na árvore (retorna offset do registro ou -1)
long btree_search(BTree *tree, Key key) {
    BTreeNode *root = btree_node_load(tree, tree->root_offset);
    long result = btree_search_node(tree, root, key);
    btree_node_free(root);
    return result;
}

// ==================== INSERÇÃO ====================

// Divide um nó filho cheio
void btree_split_child(BTree *tree, BTreeNode *parent, int index) {
    BTreeNode *full_child = btree_node_load(tree, parent->children[index]);
    BTreeNode *new_child = btree_node_create(full_child->is_leaf, tree->key_type);

    // Novo nó fica com T-1 chaves (metade superior)
    new_child->num_keys = T - 1;

    for (int j = 0; j < T - 1; j++) {
        key_copy(&new_child->keys[j], &full_child->keys[j + T]);
        new_child->offsets[j] = full_child->offsets[j + T];
    }

    // Se não é folha, copia filhos também
    if (!full_child->is_leaf) {
        for (int j = 0; j < T; j++) {
            new_child->children[j] = full_child->children[j + T];
        }
    }

    // Nó cheio fica com T-1 chaves (metade inferior)
    full_child->num_keys = T - 1;

    // Move filhos do pai para abrir espaço
    for (int j = parent->num_keys; j > index; j--) {
        parent->children[j + 1] = parent->children[j];
    }

    // Salva novo nó e atualiza ponteiro no pai
    btree_node_save(tree, new_child);
    parent->children[index + 1] = new_child->offset_self;

    // Move chaves do pai para abrir espaço
    for (int j = parent->num_keys - 1; j >= index; j--) {
        key_copy(&parent->keys[j + 1], &parent->keys[j]);
        parent->offsets[j + 1] = parent->offsets[j];
    }

    // Move chave do meio do nó cheio para o pai
    key_copy(&parent->keys[index], &full_child->keys[T - 1]);
    parent->offsets[index] = full_child->offsets[T - 1];
    parent->num_keys++;

    // Salva nós modificados
    btree_node_save(tree, full_child);
    btree_node_save(tree, parent);

    btree_node_free(full_child);
    btree_node_free(new_child);
}

// Insere em um nó não cheio
void btree_insert_nonfull(BTree *tree, BTreeNode *node, Key key, long offset) {
    int i = node->num_keys - 1;

    if (node->is_leaf) {
        // Insere diretamente na folha
        while (i >= 0 && key_compare(&key, &node->keys[i]) < 0) {
            key_copy(&node->keys[i + 1], &node->keys[i]);
            node->offsets[i + 1] = node->offsets[i];
            i--;
        }

        key_copy(&node->keys[i + 1], &key);
        node->offsets[i + 1] = offset;
        node->num_keys++;

        btree_node_save(tree, node);
    } else {
        // Encontra filho onde inserir
        while (i >= 0 && key_compare(&key, &node->keys[i]) < 0) {
            i--;
        }
        i++;

        BTreeNode *child = btree_node_load(tree, node->children[i]);

        // Se filho está cheio, divide
        if (child->num_keys == MAX_KEYS) {
            btree_split_child(tree, node, i);

            // Atualiza node após split
            long node_offset = node->offset_self;
            btree_node_free(node);
            node = btree_node_load(tree, node_offset);

            if (key_compare(&key, &node->keys[i]) > 0) {
                i++;
            }

            btree_node_free(child);
            child = btree_node_load(tree, node->children[i]);
        }

        btree_insert_nonfull(tree, child, key, offset);
        btree_node_free(child);
    }
}

// Insere uma chave na árvore
bool btree_insert(BTree *tree, Key key, long offset) {
    // Verifica se chave já existe
    if (btree_search(tree, key) >= 0) {
        fprintf(stderr, "Erro: chave já existe na árvore\n");
        return false;
    }

    BTreeNode *root = btree_node_load(tree, tree->root_offset);

    // Se raiz está cheia, cria nova raiz
    if (root->num_keys == MAX_KEYS) {
        BTreeNode *new_root = btree_node_create(false, tree->key_type);
        new_root->children[0] = tree->root_offset;

        btree_split_child(tree, new_root, 0);

        btree_node_save(tree, new_root);
        tree->root_offset = new_root->offset_self;

        // Atualiza offset da raiz no arquivo
        fseek(tree->idx_file, 0, SEEK_SET);
        fwrite(&tree->root_offset, sizeof(long), 1, tree->idx_file);
        fflush(tree->idx_file);

        btree_insert_nonfull(tree, new_root, key, offset);
        btree_node_free(new_root);
    } else {
        btree_insert_nonfull(tree, root, key, offset);
    }

    btree_node_free(root);
    return true;
}

// ==================== REMOÇÃO ====================

// Junta um nó com seu irmão
void btree_merge(BTree *tree, BTreeNode *node, int index) {
    BTreeNode *child = btree_node_load(tree, node->children[index]);
    BTreeNode *sibling = btree_node_load(tree, node->children[index + 1]);

    // Puxa chave do pai para o filho
    key_copy(&child->keys[T - 1], &node->keys[index]);
    child->offsets[T - 1] = node->offsets[index];

    // Copia chaves do irmão para o filho
    for (int i = 0; i < sibling->num_keys; i++) {
        key_copy(&child->keys[i + T], &sibling->keys[i]);
        child->offsets[i + T] = sibling->offsets[i];
    }

    // Copia filhos do irmão (se não for folha)
    if (!child->is_leaf) {
        for (int i = 0; i <= sibling->num_keys; i++) {
            child->children[i + T] = sibling->children[i];
        }
    }

    // Atualiza número de chaves do filho
    child->num_keys += sibling->num_keys + 1;

    // Move chaves do pai
    for (int i = index + 1; i < node->num_keys; i++) {
        key_copy(&node->keys[i - 1], &node->keys[i]);
        node->offsets[i - 1] = node->offsets[i];
    }

    // Move filhos do pai
    for (int i = index + 2; i <= node->num_keys; i++) {
        node->children[i - 1] = node->children[i];
    }

    node->num_keys--;

    btree_node_save(tree, child);
    btree_node_save(tree, node);

    btree_node_free(child);
    btree_node_free(sibling);
}

// Pega uma chave do irmão anterior
void btree_borrow_from_prev(BTree *tree, BTreeNode *node, int index) {
    BTreeNode *child = btree_node_load(tree, node->children[index]);
    BTreeNode *sibling = btree_node_load(tree, node->children[index - 1]);

    // Move todas as chaves do filho uma posição à frente
    for (int i = child->num_keys - 1; i >= 0; i--) {
        key_copy(&child->keys[i + 1], &child->keys[i]);
        child->offsets[i + 1] = child->offsets[i];
    }

    // Move filhos se não for folha
    if (!child->is_leaf) {
        for (int i = child->num_keys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    // Move chave do pai para o filho
    key_copy(&child->keys[0], &node->keys[index - 1]);
    child->offsets[0] = node->offsets[index - 1];

    // Move última chave do irmão para o pai
    key_copy(&node->keys[index - 1], &sibling->keys[sibling->num_keys - 1]);
    node->offsets[index - 1] = sibling->offsets[sibling->num_keys - 1];

    // Move último filho do irmão (se não for folha)
    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }

    child->num_keys++;
    sibling->num_keys--;

    btree_node_save(tree, child);
    btree_node_save(tree, sibling);
    btree_node_save(tree, node);

    btree_node_free(child);
    btree_node_free(sibling);
}

// Pega uma chave do próximo irmão
void btree_borrow_from_next(BTree *tree, BTreeNode *node, int index) {
    BTreeNode *child = btree_node_load(tree, node->children[index]);
    BTreeNode *sibling = btree_node_load(tree, node->children[index + 1]);

    // Move chave do pai para o filho
    key_copy(&child->keys[child->num_keys], &node->keys[index]);
    child->offsets[child->num_keys] = node->offsets[index];

    // Move primeiro filho do irmão (se não for folha)
    if (!child->is_leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
    }

    // Move primeira chave do irmão para o pai
    key_copy(&node->keys[index], &sibling->keys[0]);
    node->offsets[index] = sibling->offsets[0];

    // Move todas as chaves do irmão uma posição para trás
    for (int i = 1; i < sibling->num_keys; i++) {
        key_copy(&sibling->keys[i - 1], &sibling->keys[i]);
        sibling->offsets[i - 1] = sibling->offsets[i];
    }

    // Move filhos do irmão (se não for folha)
    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->num_keys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->num_keys++;
    sibling->num_keys--;

    btree_node_save(tree, child);
    btree_node_save(tree, sibling);
    btree_node_save(tree, node);

    btree_node_free(child);
    btree_node_free(sibling);
}

// Preenche um filho que tem menos de T-1 chaves
void btree_fill(BTree *tree, BTreeNode *node, int index) {
    // Se irmão anterior tem >= T chaves, pega dele
    if (index != 0) {
        BTreeNode *prev_sibling = btree_node_load(tree, node->children[index - 1]);
        if (prev_sibling->num_keys >= T) {
            btree_node_free(prev_sibling);
            btree_borrow_from_prev(tree, node, index);
            return;
        }
        btree_node_free(prev_sibling);
    }

    // Se próximo irmão tem >= T chaves, pega dele
    if (index != node->num_keys) {
        BTreeNode *next_sibling = btree_node_load(tree, node->children[index + 1]);
        if (next_sibling->num_keys >= T) {
            btree_node_free(next_sibling);
            btree_borrow_from_next(tree, node, index);
            return;
        }
        btree_node_free(next_sibling);
    }

    // Junta com irmão
    if (index != node->num_keys) {
        btree_merge(tree, node, index);
    } else {
        btree_merge(tree, node, index - 1);
    }
}

// Remove uma chave de um nó
void btree_delete_from_node(BTree *tree, BTreeNode *node, Key key) {
    int i = 0;

    // Encontra a primeira chave >= key
    while (i < node->num_keys && key_compare(&key, &node->keys[i]) > 0) {
        i++;
    }

    // Caso 1: chave está neste nó
    if (i < node->num_keys && key_equals(&node->keys[i], &key)) {
        if (node->is_leaf) {
            // Remove da folha
            for (int j = i + 1; j < node->num_keys; j++) {
                key_copy(&node->keys[j - 1], &node->keys[j]);
                node->offsets[j - 1] = node->offsets[j];
            }
            node->num_keys--;
            btree_node_save(tree, node);
        } else {
            // Nó interno: substituir por predecessor ou sucessor
            BTreeNode *pred = btree_node_load(tree, node->children[i]);
            if (pred->num_keys >= T) {
                // Pega predecessor (maior chave da subárvore esquerda)
                while (!pred->is_leaf) {
                    BTreeNode *tmp = btree_node_load(tree, pred->children[pred->num_keys]);
                    btree_node_free(pred);
                    pred = tmp;
                }

                key_copy(&node->keys[i], &pred->keys[pred->num_keys - 1]);
                node->offsets[i] = pred->offsets[pred->num_keys - 1];
                btree_node_save(tree, node);

                Key pred_key = pred->keys[pred->num_keys - 1];
                btree_node_free(pred);

                BTreeNode *left_child = btree_node_load(tree, node->children[i]);
                btree_delete_from_node(tree, left_child, pred_key);
                btree_node_free(left_child);
            } else {
                BTreeNode *succ = btree_node_load(tree, node->children[i + 1]);
                btree_node_free(pred);

                if (succ->num_keys >= T) {
                    // Pega sucessor (menor chave da subárvore direita)
                    while (!succ->is_leaf) {
                        BTreeNode *tmp = btree_node_load(tree, succ->children[0]);
                        btree_node_free(succ);
                        succ = tmp;
                    }

                    key_copy(&node->keys[i], &succ->keys[0]);
                    node->offsets[i] = succ->offsets[0];
                    btree_node_save(tree, node);

                    Key succ_key = succ->keys[0];
                    btree_node_free(succ);

                    BTreeNode *right_child = btree_node_load(tree, node->children[i + 1]);
                    btree_delete_from_node(tree, right_child, succ_key);
                    btree_node_free(right_child);
                } else {
                    // Junta e remove
                    btree_node_free(succ);
                    btree_merge(tree, node, i);

                    BTreeNode *merged_child = btree_node_load(tree, node->children[i]);
                    btree_delete_from_node(tree, merged_child, key);
                    btree_node_free(merged_child);
                }
            }
        }
    } else {
        // Caso 2: chave não está neste nó
        if (node->is_leaf) {
            // Chave não existe
            return;
        }

        bool is_in_last_child = (i == node->num_keys);

        BTreeNode *child = btree_node_load(tree, node->children[i]);

        // Se filho tem apenas T-1 chaves, preenche
        if (child->num_keys < T) {
            btree_node_free(child);
            btree_fill(tree, node, i);

            // Atualiza node após fill
            long node_offset = node->offset_self;
            btree_node_free(node);
            node = btree_node_load(tree, node_offset);

            if (is_in_last_child && i > node->num_keys) {
                i--;
            }

            child = btree_node_load(tree, node->children[i]);
        }

        btree_delete_from_node(tree, child, key);
        btree_node_free(child);
    }
}

// Remove uma chave da árvore
bool btree_delete(BTree *tree, Key key) {
    // Verifica se chave existe
    if (btree_search(tree, key) < 0) {
        return false;
    }

    BTreeNode *root = btree_node_load(tree, tree->root_offset);

    btree_delete_from_node(tree, root, key);

    // Se raiz ficou vazia, faz primeiro filho virar raiz
    btree_node_free(root);
    root = btree_node_load(tree, tree->root_offset);

    if (root->num_keys == 0) {
        if (!root->is_leaf) {
            tree->root_offset = root->children[0];

            // Atualiza offset da raiz no arquivo
            fseek(tree->idx_file, 0, SEEK_SET);
            fwrite(&tree->root_offset, sizeof(long), 1, tree->idx_file);
            fflush(tree->idx_file);
        }
    }

    btree_node_free(root);
    return true;
}

// ==================== TRAVESSIA ====================

// Adiciona par chave-offset à lista (expande se necessário)
static void add_pair_to_list(KeyOffsetList *list, Key key, long offset) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->pairs = realloc(list->pairs, list->capacity * sizeof(KeyOffsetPair));
    }
    list->pairs[list->count].key = key;
    list->pairs[list->count].offset = offset;
    list->count++;
}

// Coleta recursivamente todos os pares da árvore
static void collect_pairs(BTree *tree, BTreeNode *node, KeyOffsetList *list) {
    if (!node) return;

    int i;
    for (i = 0; i < node->num_keys; i++) {
        // Processa filho esquerdo primeiro (travessia em ordem)
        if (!node->is_leaf) {
            BTreeNode *child = btree_node_load(tree, node->children[i]);
            if (child) {
                collect_pairs(tree, child, list);
                btree_node_free(child);
            }
        }

        // Adiciona chave atual
        add_pair_to_list(list, node->keys[i], node->offsets[i]);
    }

    // Processa último filho
    if (!node->is_leaf) {
        BTreeNode *child = btree_node_load(tree, node->children[i]);
        if (child) {
            collect_pairs(tree, child, list);
            btree_node_free(child);
        }
    }
}

// Retorna lista com todos os pares chave-offset da árvore
KeyOffsetList* btree_get_all_pairs(BTree *tree) {
    KeyOffsetList *list = malloc(sizeof(KeyOffsetList));
    list->capacity = 100;
    list->count = 0;
    list->pairs = malloc(list->capacity * sizeof(KeyOffsetPair));

    BTreeNode *root = btree_node_load(tree, tree->root_offset);
    collect_pairs(tree, root, list);
    btree_node_free(root);

    return list;
}

// Libera memória da lista
void keyoffset_list_free(KeyOffsetList *list) {
    if (list) {
        free(list->pairs);
        free(list);
    }
}
