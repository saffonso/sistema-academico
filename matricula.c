#include "matricula.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variável global para acesso nas funções de cascade
MatriculaManager *global_matricula_manager = NULL;

// Cria um novo gerenciador de matrículas
MatriculaManager* matricula_manager_create(const char *dat_filename, const char *idx_filename,
                                          AlunoManager *aluno_manager, DisciplinaManager *disciplina_manager) {
    MatriculaManager *manager = (MatriculaManager*)malloc(sizeof(MatriculaManager));
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

    // Armazenar referências para validação de FK
    manager->aluno_manager = aluno_manager;
    manager->disciplina_manager = disciplina_manager;

    // Configurar gerenciador global
    global_matricula_manager = manager;

    return manager;
}

// Abre um gerenciador de matrículas existente
MatriculaManager* matricula_manager_open(const char *dat_filename, const char *idx_filename,
                                        AlunoManager *aluno_manager, DisciplinaManager *disciplina_manager) {
    MatriculaManager *manager = (MatriculaManager*)malloc(sizeof(MatriculaManager));
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

    // Armazenar referências para validação de FK
    manager->aluno_manager = aluno_manager;
    manager->disciplina_manager = disciplina_manager;

    // Configurar gerenciador global
    global_matricula_manager = manager;

    return manager;
}

// Fecha o gerenciador de matrículas
void matricula_manager_close(MatriculaManager *manager) {
    if (manager) {
        if (manager->dat_file) {
            fclose(manager->dat_file);
        }
        if (manager->idx_tree) {
            btree_close(manager->idx_tree);
        }
        if (global_matricula_manager == manager) {
            global_matricula_manager = NULL;
        }
        free(manager);
    }
}

// Salva um registro de matrícula no arquivo .dat e retorna o offset
long matricula_salvar_registro(MatriculaManager *manager, Matricula *matricula) {
    if (!manager || !manager->dat_file || !matricula) {
        return -1;
    }

    // Vai para o final do arquivo
    fseek(manager->dat_file, 0, SEEK_END);
    long offset = ftell(manager->dat_file);

    // Escreve o registro
    size_t written = fwrite(matricula, sizeof(Matricula), 1, manager->dat_file);
    fflush(manager->dat_file);

    if (written != 1) {
        return -1;
    }

    return offset;
}

// Carrega um registro de matrícula do arquivo .dat
Matricula* matricula_carregar_registro(MatriculaManager *manager, long offset) {
    if (!manager || !manager->dat_file || offset < 0) {
        return NULL;
    }

    Matricula *matricula = (Matricula*)malloc(sizeof(Matricula));
    if (!matricula) {
        return NULL;
    }

    // Vai para o offset e lê o registro
    fseek(manager->dat_file, offset, SEEK_SET);
    size_t read = fread(matricula, sizeof(Matricula), 1, manager->dat_file);

    if (read != 1) {
        free(matricula);
        return NULL;
    }

    return matricula;
}

// Cria uma nova matrícula (com validação de FK)
bool matricula_criar(MatriculaManager *manager, Matricula *matricula) {
    if (!manager || !matricula) {
        return false;
    }

    // Validar FK: aluno deve existir
    if (!aluno_existe(manager->aluno_manager, matricula->matricula_aluno)) {
        printf("Erro: Aluno com matrícula %d não existe.\n", matricula->matricula_aluno);
        return false;
    }

    // Validar FK: disciplina deve existir
    if (!disciplina_existe(manager->disciplina_manager, matricula->codigo_disciplina)) {
        printf("Erro: Disciplina com código %s não existe.\n", matricula->codigo_disciplina);
        return false;
    }

    // Verifica se a matrícula já existe
    Key key = create_int_key(matricula->id_matricula);
    long existing_offset = btree_search(manager->idx_tree, key);
    if (existing_offset != -1) {
        printf("Erro: Matrícula com ID %d já existe.\n", matricula->id_matricula);
        return false;
    }

    // Salva o registro no arquivo .dat
    long offset = matricula_salvar_registro(manager, matricula);
    if (offset == -1) {
        printf("Erro ao salvar registro da matrícula.\n");
        return false;
    }

    // Insere na árvore B
    if (!btree_insert(manager->idx_tree, key, offset)) {
        printf("Erro ao inserir matrícula no índice.\n");
        return false;
    }

    return true;
}

// Busca uma matrícula por ID
Matricula* matricula_buscar(MatriculaManager *manager, int id_matricula) {
    if (!manager) {
        return NULL;
    }

    // Busca na árvore B
    Key key = create_int_key(id_matricula);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        return NULL;
    }

    // Carrega o registro do arquivo .dat
    return matricula_carregar_registro(manager, offset);
}

// Atualiza apenas a média final de uma matrícula
bool matricula_atualizar_media(MatriculaManager *manager, int id_matricula, float nova_media) {
    if (!manager) {
        return false;
    }

    // Busca o offset da matrícula na árvore B
    Key key = create_int_key(id_matricula);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        printf("Erro: Matrícula com ID %d não encontrada.\n", id_matricula);
        return false;
    }

    // Carrega o registro
    Matricula *matricula = matricula_carregar_registro(manager, offset);
    if (!matricula) {
        printf("Erro ao carregar matrícula.\n");
        return false;
    }

    // Atualiza apenas a média final
    matricula->media_final = nova_media;

    // Salva de volta no mesmo offset
    fseek(manager->dat_file, offset, SEEK_SET);
    size_t written = fwrite(matricula, sizeof(Matricula), 1, manager->dat_file);
    fflush(manager->dat_file);

    free(matricula);

    if (written != 1) {
        printf("Erro ao atualizar matrícula.\n");
        return false;
    }

    return true;
}

// Verifica se uma matrícula existe
bool matricula_existe(MatriculaManager *manager, int id_matricula) {
    if (!manager) {
        return false;
    }

    Key key = create_int_key(id_matricula);
    long offset = btree_search(manager->idx_tree, key);
    return (offset != -1);
}

// Deleta uma matrícula
bool matricula_deletar(MatriculaManager *manager, int id_matricula) {
    if (!manager) {
        return false;
    }

    // Verifica se a matrícula existe
    Key key = create_int_key(id_matricula);
    long offset = btree_search(manager->idx_tree, key);

    if (offset == -1) {
        printf("Erro: Matrícula com ID %d não encontrada.\n", id_matricula);
        return false;
    }

    // Remove da árvore B
    if (!btree_delete(manager->idx_tree, key)) {
        printf("Erro ao remover matrícula do índice.\n");
        return false;
    }

    return true;
}

// Lista todas as matrículas
void matricula_listar_todos(MatriculaManager *manager) {
    if (!manager) return;

    printf("\n=== Lista de Matrículas ===\n");

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    if (list->count == 0) {
        printf("Nenhuma matrícula encontrada.\n");
    } else {
        for (int i = 0; i < list->count; i++) {
            Matricula *m = matricula_carregar_registro(manager, list->pairs[i].offset);
            if (m) {
                printf("ID: %d | Aluno: %d | Disciplina: %s | Media: %.2f\n",
                       m->id_matricula, m->matricula_aluno, m->codigo_disciplina, m->media_final);
                free(m);
            }
        }
    }
    printf("\n");
    keyoffset_list_free(list);
}

// Conta o número de matrículas
int matricula_contar(MatriculaManager *manager) {
    if (!manager) return 0;

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);
    int count = list->count;
    keyoffset_list_free(list);
    return count;
}

// Lista matrículas de um aluno específico
void matricula_listar_por_aluno(MatriculaManager *manager, int matricula_aluno) {
    if (!manager) return;

    printf("\n=== Matrículas do Aluno %d ===\n", matricula_aluno);

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    for (int i = 0; i < list->count; i++) {
        Matricula *m = matricula_carregar_registro(manager, list->pairs[i].offset);
        if (m && m->matricula_aluno == matricula_aluno) {
            printf("ID: %d | Disciplina: %s | Ano: %d/%d | Media: %.2f\n",
                   m->id_matricula, m->codigo_disciplina,
                   m->ano_letivo, m->semestre_letivo, m->media_final);
        }
        if (m) free(m);
    }

    printf("\n");
    keyoffset_list_free(list);
}

// Lista matrículas de uma disciplina específica
void matricula_listar_por_disciplina(MatriculaManager *manager, const char *codigo_disciplina) {
    if (!manager || !codigo_disciplina) return;

    printf("\n=== Alunos na Disciplina %s ===\n", codigo_disciplina);

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    for (int i = 0; i < list->count; i++) {
        Matricula *m = matricula_carregar_registro(manager, list->pairs[i].offset);
        if (m && strcmp(m->codigo_disciplina, codigo_disciplina) == 0) {
            printf("ID: %d | Aluno: %d | Ano: %d/%d | Media: %.2f\n",
                   m->id_matricula, m->matricula_aluno,
                   m->ano_letivo, m->semestre_letivo, m->media_final);
        }
        if (m) free(m);
    }

    printf("\n");
    keyoffset_list_free(list);
}

// Lista histórico completo de um aluno
void matricula_listar_historico_aluno(MatriculaManager *manager, int matricula_aluno,
                                      AlunoManager *aluno_mgr, DisciplinaManager *disciplina_mgr) {
    if (!manager || !aluno_mgr || !disciplina_mgr) return;

    printf("\nDisciplinas cursadas:\n");
    printf("-------------------------------------------\n");

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    for (int i = 0; i < list->count; i++) {
        Matricula *m = matricula_carregar_registro(manager, list->pairs[i].offset);
        if (m && m->matricula_aluno == matricula_aluno) {
            Disciplina *disc = disciplina_buscar(disciplina_mgr, m->codigo_disciplina);
            if (disc) {
                printf("%s - %s | %d/%d | Media: %.2f\n",
                       m->codigo_disciplina, disc->nome_disciplina,
                       m->ano_letivo, m->semestre_letivo, m->media_final);
                free(disc);
            }
        }
        if (m) free(m);
    }

    keyoffset_list_free(list);
}

// Lista alunos em uma disciplina
void matricula_listar_alunos_em_disciplina(MatriculaManager *manager, const char *codigo_disciplina,
                                           AlunoManager *aluno_mgr, DisciplinaManager *disciplina_mgr) {
    if (!manager || !aluno_mgr || !disciplina_mgr || !codigo_disciplina) return;

    printf("\nAlunos matriculados:\n");
    printf("-------------------------------------------\n");

    KeyOffsetList *list = btree_get_all_pairs(manager->idx_tree);

    for (int i = 0; i < list->count; i++) {
        Matricula *m = matricula_carregar_registro(manager, list->pairs[i].offset);
        if (m && strcmp(m->codigo_disciplina, codigo_disciplina) == 0) {
            Aluno *aluno = aluno_buscar(aluno_mgr, m->matricula_aluno);
            if (aluno) {
                printf("Matricula: %d | Nome: %s | Media: %.2f\n",
                       aluno->matricula, aluno->nome_aluno, m->media_final);
                free(aluno);
            }
        }
        if (m) free(m);
    }

    keyoffset_list_free(list);
}

// Cascade delete: remove todas as matrículas de um aluno
bool matricula_deletar_por_aluno(int matricula_aluno) {
    if (!global_matricula_manager) return false;

    KeyOffsetList *list = btree_get_all_pairs(global_matricula_manager->idx_tree);

    // Coleta IDs das matrículas do aluno
    int ids_to_delete[1000];
    int count = 0;

    for (int i = 0; i < list->count && count < 1000; i++) {
        Matricula *m = matricula_carregar_registro(global_matricula_manager, list->pairs[i].offset);
        if (m && m->matricula_aluno == matricula_aluno) {
            ids_to_delete[count++] = m->id_matricula;
        }
        if (m) free(m);
    }

    keyoffset_list_free(list);

    // Deleta cada matrícula coletada
    for (int i = 0; i < count; i++) {
        matricula_deletar(global_matricula_manager, ids_to_delete[i]);
    }

    return true;
}

// Cascade delete: remove todas as matrículas de uma disciplina
bool matricula_deletar_por_disciplina(const char *codigo_disciplina) {
    if (!global_matricula_manager || !codigo_disciplina) return false;

    KeyOffsetList *list = btree_get_all_pairs(global_matricula_manager->idx_tree);

    // Coleta IDs das matrículas da disciplina
    int ids_to_delete[1000];
    int count = 0;

    for (int i = 0; i < list->count && count < 1000; i++) {
        Matricula *m = matricula_carregar_registro(global_matricula_manager, list->pairs[i].offset);
        if (m && strcmp(m->codigo_disciplina, codigo_disciplina) == 0) {
            ids_to_delete[count++] = m->id_matricula;
        }
        if (m) free(m);
    }

    keyoffset_list_free(list);

    // Deleta cada matrícula coletada
    for (int i = 0; i < count; i++) {
        matricula_deletar(global_matricula_manager, ids_to_delete[i]);
    }

    return true;
}
