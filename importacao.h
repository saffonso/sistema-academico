#ifndef IMPORTACAO_H
#define IMPORTACAO_H

#include "aluno.h"
#include "disciplina.h"
#include "matricula.h"
#include <stdbool.h>

// Função para importar alunos de um arquivo CSV
int importar_alunos(const char *csv_filename, AlunoManager *manager);

// Função para importar disciplinas de um arquivo CSV
int importar_disciplinas(const char *csv_filename, DisciplinaManager *manager);

// Função para importar matrículas de um arquivo CSV
// Gera valores aleatórios para media_final quando campo estiver vazio
int importar_matriculas(const char *csv_filename, MatriculaManager *manager);

// Função auxiliar para remover caracteres de nova linha
void remover_newline(char *str);

// Função auxiliar para extrair campo de CSV (lida com vírgulas e aspas)
char *extrair_campo_csv(char **linha);

// Função auxiliar para gerar média aleatória entre 0.0 e 10.0
float gerar_media_aleatoria();

#endif
