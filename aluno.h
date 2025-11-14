#ifndef ALUNO_H
#define ALUNO_H

#include "btree.h"
#include <stdbool.h>

// Estrutura de um Aluno
typedef struct {
    int matricula;                  // Chave primária
    char nome_aluno[100];
    char endereco[150];
    char telefone[20];
    int ano_ingresso;
    int semestre_ingresso;
    char data_nascimento[15];       // Formato: DD/MM/AAAA
} Aluno;

// Estrutura de controle dos alunos
typedef struct {
    FILE *dat_file;                 // Arquivo de dados (.dat)
    BTree *idx_tree;                // Árvore B de índice
} AlunoManager;

// Funções de gerenciamento
AlunoManager* aluno_manager_create(const char *dat_filename, const char *idx_filename);
AlunoManager* aluno_manager_open(const char *dat_filename, const char *idx_filename);
void aluno_manager_close(AlunoManager *manager);

// Operações CRUD
bool aluno_criar(AlunoManager *manager, Aluno *aluno);
Aluno* aluno_buscar(AlunoManager *manager, int matricula);
bool aluno_atualizar(AlunoManager *manager, Aluno *aluno);
bool aluno_deletar(AlunoManager *manager, int matricula);

// Operações auxiliares
void aluno_listar_todos(AlunoManager *manager);
int aluno_contar(AlunoManager *manager);
bool aluno_existe(AlunoManager *manager, int matricula);
long aluno_salvar_registro(AlunoManager *manager, Aluno *aluno);
Aluno* aluno_carregar_registro(AlunoManager *manager, long offset);

#endif // ALUNO_H
