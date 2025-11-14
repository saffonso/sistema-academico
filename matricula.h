#ifndef MATRICULA_H
#define MATRICULA_H

#include "btree.h"
#include "aluno.h"
#include "disciplina.h"
#include <stdbool.h>

// Estrutura de uma Matrícula
typedef struct {
    int id_matricula;               // Chave primária
    int matricula_aluno;            // FK para Aluno
    char codigo_disciplina[20];     // FK para Disciplina
    int ano_letivo;
    int semestre_letivo;
    float media_final;
} Matricula;

// Estrutura de controle das matrículas
typedef struct {
    FILE *dat_file;                 // Arquivo de dados (.dat)
    BTree *idx_tree;                // Árvore B de índice
    AlunoManager *aluno_manager;    // Referência para validação de FK
    DisciplinaManager *disciplina_manager; // Referência para validação de FK
} MatriculaManager;

// Funções de gerenciamento
MatriculaManager* matricula_manager_create(const char *dat_filename, const char *idx_filename,
                                          AlunoManager *aluno_manager, DisciplinaManager *disciplina_manager);
MatriculaManager* matricula_manager_open(const char *dat_filename, const char *idx_filename,
                                        AlunoManager *aluno_manager, DisciplinaManager *disciplina_manager);
void matricula_manager_close(MatriculaManager *manager);

// Operações CRUD
bool matricula_criar(MatriculaManager *manager, Matricula *matricula);
Matricula* matricula_buscar(MatriculaManager *manager, int id_matricula);
bool matricula_atualizar_media(MatriculaManager *manager, int id_matricula, float nova_media);
bool matricula_deletar(MatriculaManager *manager, int id_matricula);

// Operações auxiliares
void matricula_listar_todos(MatriculaManager *manager);
int matricula_contar(MatriculaManager *manager);
bool matricula_existe(MatriculaManager *manager, int id_matricula);
long matricula_salvar_registro(MatriculaManager *manager, Matricula *matricula);
Matricula* matricula_carregar_registro(MatriculaManager *manager, long offset);

// Funções de cascade delete (chamadas por aluno e disciplina)
bool matricula_deletar_por_aluno(int matricula_aluno);
bool matricula_deletar_por_disciplina(const char *codigo_disciplina);

// Funções de listagem filtrada
void matricula_listar_por_aluno(MatriculaManager *manager, int matricula_aluno);
void matricula_listar_por_disciplina(MatriculaManager *manager, const char *codigo_disciplina);

// Funções de relatório
void matricula_listar_historico_aluno(MatriculaManager *manager, int matricula_aluno,
                                      AlunoManager *aluno_mgr, DisciplinaManager *disciplina_mgr);
void matricula_listar_alunos_em_disciplina(MatriculaManager *manager, const char *codigo_disciplina,
                                           AlunoManager *aluno_mgr, DisciplinaManager *disciplina_mgr);

// Variável global para acesso nas funções de cascade
extern MatriculaManager *global_matricula_manager;

#endif // MATRICULA_H
