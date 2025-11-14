#ifndef DISCIPLINA_H
#define DISCIPLINA_H

#include "btree.h"
#include <stdbool.h>

// Estrutura de uma Disciplina
typedef struct {
    char codigo_disciplina[20];     // Chave primária
    char nome_disciplina[100];
} Disciplina;

// Estrutura de controle das disciplinas
typedef struct {
    FILE *dat_file;                 // Arquivo de dados (.dat)
    BTree *idx_tree;                // Árvore B de índice
} DisciplinaManager;

// Funções de gerenciamento
DisciplinaManager* disciplina_manager_create(const char *dat_filename, const char *idx_filename);
DisciplinaManager* disciplina_manager_open(const char *dat_filename, const char *idx_filename);
void disciplina_manager_close(DisciplinaManager *manager);

// Operações CRUD
bool disciplina_criar(DisciplinaManager *manager, Disciplina *disciplina);
Disciplina* disciplina_buscar(DisciplinaManager *manager, const char *codigo_disciplina);
bool disciplina_atualizar(DisciplinaManager *manager, Disciplina *disciplina);
bool disciplina_deletar(DisciplinaManager *manager, const char *codigo_disciplina);

// Operações auxiliares
void disciplina_listar_todos(DisciplinaManager *manager);
int disciplina_contar(DisciplinaManager *manager);
bool disciplina_existe(DisciplinaManager *manager, const char *codigo_disciplina);
long disciplina_salvar_registro(DisciplinaManager *manager, Disciplina *disciplina);
Disciplina* disciplina_carregar_registro(DisciplinaManager *manager, long offset);

#endif // DISCIPLINA_H
