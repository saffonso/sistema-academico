#include "disciplina.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration para evitar dependência circular
bool matricula_deletar_por_disciplina(const char *codigo_disciplina);

// Cria um novo gerenciador de disciplinas
DisciplinaManager* disciplina_manager_create(const char *dat_filename, const char *idx_filename) {
    DisciplinaManager *manager = (DisciplinaManager*)malloc(sizeof(DisciplinaManager));
    if (!manager) {
        return NULL;
    }

    // Criar arquivo de dados
    manager->dat_file = fopen(dat_filename, "wb+");
    if (!manager->dat_file) {
        free(manager);
        return NULL;
    }

    // Criar árvore B de índice (chaves string)
    manager->idx_tree = btree_create(idx_filename, KEY_STRING);
    if (!manager->idx_tree) {
        fclose(manager->dat_file);
        free(manager);
        return NULL;
    }

    return manager;
}

// Abre um gerenciador de disciplinas existente
DisciplinaManager* disciplina_manager_open(const char *dat_filename, const char *idx_filename) {
    DisciplinaManager *manager = (DisciplinaManager*)malloc(sizeof(DisciplinaManager));
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

// Fecha o gerenciador de disciplinas
void disciplina_manager_close(DisciplinaManager *manager) {
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

// Salva um registro de disciplina no arquivo .dat e retorna o offset
long disciplina_salvar_registro(DisciplinaManager *manager, Disciplina *disciplina) {
    if (!manager || !manager->dat_file || !disciplina) {
        return -1;
    }

    // Vai para o final do arquivo
    fseek(manager->dat_file, 0, SEEK_END);
    long offset = ftell(manager->dat_file);

    // Escreve o registro
    size_t written = fwrite(disciplina, sizeof(Disciplina), 1, manager->dat_file);
    fflush(manager->dat_file);

    if (written != 1) {
        return -1;
    }

    return offset;
}

// Carrega um registro de disciplina do arquivo .dat
Disciplina* disciplina_carregar_registro(DisciplinaManager *manager, long offset) {
    if (!manager || !manager->dat_file || offset < 0) {
        return NULL;
    }

    Disciplina *disciplina = (Disciplina*)malloc(sizeof(Disciplina));
    if (!disciplina) {
        return NULL;
    }

    // Vai para o offset e lê o registro
    fseek(manager->dat_file, offset, SEEK_SET);
    size_t read = fread(disciplina, sizeof(Disciplina), 1, manager->dat_file);

    if (read != 1) {
        free(disciplina);
        return NULL;
    }

    return disciplina;
}

// Cria uma nova disciplina
bool disciplina_criar(DisciplinaManager *manager, Disciplina *disciplina) {
    if (!manager || !disciplina) {
        return false;
    }

    // Verifica se a disciplina já existe
    Key key = create_string_key(disciplina->codigo_disciplina);
    long existing_offset = btree_search(manager->idx_tree, key);
    if (existing_offset != -1) {
        printf("Erro: Disciplina com código %s já existe.\n", disciplina->codigo_disciplina);
        return false;
    }

    // Salva o registro no arquivo .dat
    long offset = disciplina_salvar_registro(manager, disciplina);
    if (offset == -1) {
        printf("Erro ao salvar registro da disciplina.\n");
        return false;
    }

    // Insere na árvore B
    if (!btree_insert(manager->idx_tree, key, offset)) {
        printf("Erro ao inserir disciplina no índice.\n");
        return false;
    }

    return true;
}

// Busca uma disciplina por código
Disciplina* disciplina_buscar(DisciplinaManager *manager, const char *codigo_disciplina) {
    if (!manager || !codigo_disciplina) {
        return NULL;
    }

    // Busca na árvore B
    Key key = create_string_key(codigo_disciplina);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        return NULL;
    }

    // Carrega o registro do arquivo .dat
    return disciplina_carregar_registro(manager, offset);
}

// Atualiza uma disciplina existente
bool disciplina_atualizar(DisciplinaManager *manager, Disciplina *disciplina) {
    if (!manager || !disciplina) {
        return false;
    }

    // Busca o offset da disciplina na árvore B
    Key key = create_string_key(disciplina->codigo_disciplina);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        printf("Erro: Disciplina com código %s não encontrada.\n", disciplina->codigo_disciplina);
        return false;
    }

    // Atualiza o registro no arquivo .dat
    fseek(manager->dat_file, offset, SEEK_SET);
    size_t written = fwrite(disciplina, sizeof(Disciplina), 1, manager->dat_file);
    fflush(manager->dat_file);

    if (written != 1) {
        printf("Erro ao atualizar registro da disciplina.\n");
        return false;
    }

    return true;
}

// Verifica se uma disciplina existe
bool disciplina_existe(DisciplinaManager *manager, const char *codigo_disciplina) {
    if (!manager || !codigo_disciplina) {
        return false;
    }

    Key key = create_string_key(codigo_disciplina);
    long offset = btree_search(manager->idx_tree, key);
    return (offset != -1);
}

// Deleta uma disciplina (com cascade para matrículas)
bool disciplina_deletar(DisciplinaManager *manager, const char *codigo_disciplina) {
    if (!manager || !codigo_disciplina) {
        return false;
    }

    // Verifica se a disciplina existe
    Key key = create_string_key(codigo_disciplina);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        printf("Erro: Disciplina com código %s não encontrada.\n", codigo_disciplina);
        return false;
    }

    // Cascade delete: remove todas as matrículas desta disciplina
    // Esta função será implementada no módulo de matrícula
    matricula_deletar_por_disciplina(codigo_disciplina);

    // Remove da árvore B
    if (!btree_delete(manager->idx_tree, key)) {
        printf("Erro ao remover disciplina do índice.\n");
        return false;
    }

    // Nota: O registro no .dat não é fisicamente removido,
    // apenas o índice é atualizado (espaço pode ser reutilizado futuramente)

    return true;
}

// Lista todas as disciplinas
void disciplina_listar_todos(DisciplinaManager *manager) {
    if (!manager) return;

    printf("\n=== Lista de Disciplinas ===\n");

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    if (list->count == 0) {
        printf("Nenhuma disciplina encontrada.\n");
    } else {
        for (int i = 0; i < list->count; i++) {
            Disciplina *d = disciplina_carregar_registro(manager, list->pairs[i].offset);
            if (d) {
                printf("Codigo: %s | Nome: %s\n", d->codigo_disciplina, d->nome_disciplina);
                free(d);
            }
        }
    }
    printf("\n");
    keyoffset_list_free(list);
}

// Conta o número de disciplinas
int disciplina_contar(DisciplinaManager *manager) {
    if (!manager) return 0;

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);
    int count = list->count;
    keyoffset_list_free(list);
    return count;
}
