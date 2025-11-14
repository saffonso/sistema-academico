#include "aluno.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration para evitar dependência circular
bool matricula_deletar_por_aluno(int matricula_aluno);

// Cria um novo gerenciador de alunos
AlunoManager* aluno_manager_create(const char *dat_filename, const char *idx_filename) {
    AlunoManager *manager = (AlunoManager*)malloc(sizeof(AlunoManager));
    if (!manager) {
        return NULL;
    }

    // Criar arquivo de dados
    manager->dat_file = fopen(dat_filename, "wb+");
    if (!manager->dat_file) {
        free(manager);
        return NULL;
    }

    // Criar árvore B de índice (chaves inteiras)
    manager->idx_tree = btree_create(idx_filename, KEY_INT);
    if (!manager->idx_tree) {
        fclose(manager->dat_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// Abre um gerenciador de alunos existente
AlunoManager* aluno_manager_open(const char *dat_filename, const char *idx_filename) {
    AlunoManager *manager = (AlunoManager*)malloc(sizeof(AlunoManager));
    if (!manager) {
        return NULL;
    }

    // Abrir arquivo de dados
    manager->dat_file = fopen(dat_filename, "rb+");
    if (!manager->dat_file) {
        free(manager);
        return NULL;
    }

    // Abrir árvore B de índice
    manager->idx_tree = btree_open(idx_filename);
    if (!manager->idx_tree) {
        fclose(manager->dat_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// Fecha o gerenciador de alunos
void aluno_manager_close(AlunoManager *manager) {
    if (manager) {
        if (manager->dat_file) {
            fclose(manager->dat_file);
        }
        if (manager->idx_tree) {
            btree_close(manager->idx_tree);
        }
        free(manager);
    }
}

// Salva um registro de aluno no arquivo .dat e retorna o offset
long aluno_salvar_registro(AlunoManager *manager, Aluno *aluno) {
    if (!manager || !manager->dat_file || !aluno) {
        return -1;
    }

    // Vai para o final do arquivo
    fseek(manager->dat_file, 0, SEEK_END);
    long offset = ftell(manager->dat_file);

    // Escreve o registro
    size_t written = fwrite(aluno, sizeof(Aluno), 1, manager->dat_file);
    fflush(manager->dat_file);

    if (written != 1) {
        return -1;
    }

    return offset;
}

// Carrega um registro de aluno do arquivo .dat
Aluno* aluno_carregar_registro(AlunoManager *manager, long offset) {
    if (!manager || !manager->dat_file || offset < 0) {
        return NULL;
    }

    Aluno *aluno = (Aluno*)malloc(sizeof(Aluno));
    if (!aluno) {
        return NULL;
    }

    // Vai para o offset e lê o registro
    fseek(manager->dat_file, offset, SEEK_SET);
    size_t read = fread(aluno, sizeof(Aluno), 1, manager->dat_file);

    if (read != 1) {
        free(aluno);
        return NULL;
    }

    return aluno;
}

// Cria um novo aluno
bool aluno_criar(AlunoManager *manager, Aluno *aluno) {
    if (!manager || !aluno) {
        return false;
    }

    // Verifica se o aluno já existe
    Key key = create_int_key(aluno->matricula);
    long existing_offset = btree_search(manager->idx_tree, key);
    if (existing_offset != -1) {
        printf("Erro: Aluno com matrícula %d já existe.\n", aluno->matricula);
        return false;
    }

    // Salva o registro no arquivo .dat
    long offset = aluno_salvar_registro(manager, aluno);
    if (offset == -1) {
        printf("Erro ao salvar registro do aluno.\n");
        return false;
    }

    // Insere na árvore B
    if (!btree_insert(manager->idx_tree, key, offset)) {
        printf("Erro ao inserir aluno no índice.\n");
        return false;
    }

    return true;
}

// Busca um aluno por matrícula
Aluno* aluno_buscar(AlunoManager *manager, int matricula) {
    if (!manager) {
        return NULL;
    }

    // Busca na árvore B
    Key key = create_int_key(matricula);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        return NULL;
    }

    // Carrega o registro do arquivo .dat
    return aluno_carregar_registro(manager, offset);
}

// Atualiza um aluno existente
bool aluno_atualizar(AlunoManager *manager, Aluno *aluno) {
    if (!manager || !aluno) {
        return false;
    }

    // Busca o offset do aluno na árvore B
    Key key = create_int_key(aluno->matricula);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        printf("Erro: Aluno com matrícula %d não encontrado.\n", aluno->matricula);
        return false;
    }

    // Atualiza o registro no arquivo .dat
    fseek(manager->dat_file, offset, SEEK_SET);
    size_t written = fwrite(aluno, sizeof(Aluno), 1, manager->dat_file);
    fflush(manager->dat_file);

    if (written != 1) {
        printf("Erro ao atualizar registro do aluno.\n");
        return false;
    }

    return true;
}

// Verifica se um aluno existe
bool aluno_existe(AlunoManager *manager, int matricula) {
    if (!manager) {
        return false;
    }

    Key key = create_int_key(matricula);
    long offset = btree_search(manager->idx_tree, key);
    return (offset != -1);
}

// Deleta um aluno (com cascade para matrículas)
bool aluno_deletar(AlunoManager *manager, int matricula) {
    if (!manager) {
        return false;
    }

    // Verifica se o aluno existe
    Key key = create_int_key(matricula);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        printf("Erro: Aluno com matrícula %d não encontrado.\n", matricula);
        return false;
    }

    // Cascade delete: remove todas as matrículas deste aluno
    // Esta função será implementada no módulo de matrícula
    matricula_deletar_por_aluno(matricula);

    // Remove da árvore B
    if (!btree_delete(manager->idx_tree, key)) {
        printf("Erro ao remover aluno do índice.\n");
        return false;
    }

    // Nota: O registro no .dat não é fisicamente removido,
    // apenas o índice é atualizado (espaço pode ser reutilizado futuramente)

    return true;
}

// Lista todos os alunos
void aluno_listar_todos(AlunoManager *manager) {
    if (!manager) return;

    printf("\n=== Lista de Alunos ===\n");
    printf("%-15s | %-30s | %-20s\n", "Matrícula", "Nome", "Telefone");
    printf("-----------------------------------------------------------------\n");

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    if (list->count == 0) {
        printf("Nenhum aluno encontrado.\n");
    } else {
        for (int i = 0; i < list->count; i++) {
            Aluno *aluno = aluno_carregar_registro(manager, list->pairs[i].offset);
            if (aluno) {
                printf("%-15d | %-30s | %-20s\n",
                       aluno->matricula, aluno->nome_aluno, aluno->telefone);
                free(aluno);
            }
        }
    }
    printf("\n");
    keyoffset_list_free(list);
}

// Conta o número de alunos
int aluno_contar(AlunoManager *manager) {
    if (!manager) return 0;

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);
    int count = list->count;
    keyoffset_list_free(list);
    return count;
}
