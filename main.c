#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aluno.h"
#include "disciplina.h"
#include "matricula.h"
#include "importacao.h"

// Função para limpar buffer de entrada
void limpar_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// Função para pausar e aguardar usuário
void pausar()
{
    printf("\nPressione ENTER para continuar...");
    limpar_buffer();
}

// Menu principal
void exibir_menu_principal()
{
    printf("\n");
    printf("===========================================\n");
    printf("   SISTEMA DE GERENCIAMENTO ACADEMICO\n");
    printf("===========================================\n");
    printf("1.  Gerenciar Alunos\n");
    printf("2.  Gerenciar Disciplinas\n");
    printf("3.  Gerenciar Matriculas\n");
    printf("4.  Consultas e Relatorios\n");
    printf("0.  Sair\n");
    printf("===========================================\n");
    printf("Escolha uma opcao: ");
}

// Menu de gerenciamento de alunos
void menu_alunos(AlunoManager *aluno_mgr)
{
    int opcao;

    do
    {
        printf("\n");
        printf("===========================================\n");
        printf("        GERENCIAMENTO DE ALUNOS\n");
        printf("===========================================\n");
        printf("1. Criar novo aluno\n");
        printf("2. Buscar aluno por matricula\n");
        printf("3. Atualizar dados do aluno\n");
        printf("4. Deletar aluno\n");
        printf("5. Listar todos os alunos\n");
        printf("0. Voltar\n");
        printf("===========================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1)
        {
            limpar_buffer();
            printf("Entrada invalida!\n");
            pausar();
            continue;
        }
        limpar_buffer();

        switch (opcao)
        {
        case 1:
        {
            // Criar novo aluno
            Aluno novo_aluno;
            printf("\n--- Criar Novo Aluno ---\n");
            printf("Matricula: ");
            scanf("%d", &novo_aluno.matricula);
            limpar_buffer();

            printf("Nome: ");
            fgets(novo_aluno.nome_aluno, sizeof(novo_aluno.nome_aluno), stdin);
            remover_newline(novo_aluno.nome_aluno);

            printf("Endereco: ");
            fgets(novo_aluno.endereco, sizeof(novo_aluno.endereco), stdin);
            remover_newline(novo_aluno.endereco);

            printf("Telefone: ");
            fgets(novo_aluno.telefone, sizeof(novo_aluno.telefone), stdin);
            remover_newline(novo_aluno.telefone);

            printf("Ano de ingresso: ");
            scanf("%d", &novo_aluno.ano_ingresso);
            limpar_buffer();

            printf("Semestre de ingresso: ");
            scanf("%d", &novo_aluno.semestre_ingresso);
            limpar_buffer();

            printf("Data de nascimento (DD/MM/AAAA): ");
            fgets(novo_aluno.data_nascimento, sizeof(novo_aluno.data_nascimento), stdin);
            remover_newline(novo_aluno.data_nascimento);

            if (aluno_criar(aluno_mgr, &novo_aluno))
            {
                printf("Aluno criado com sucesso!\n");
            }
            else
            {
                printf("Erro ao criar aluno.\n");
            }
            pausar();
            break;
        }

        case 2:
        {
            // Buscar aluno
            int matricula;
            printf("\n--- Buscar Aluno ---\n");
            printf("Matricula: ");
            scanf("%d", &matricula);
            limpar_buffer();

            Aluno *aluno = aluno_buscar(aluno_mgr, matricula);
            if (aluno)
            {
                printf("\n--- Dados do Aluno ---\n");
                printf("Matricula: %d\n", aluno->matricula);
                printf("Nome: %s\n", aluno->nome_aluno);
                printf("Endereco: %s\n", aluno->endereco);
                printf("Telefone: %s\n", aluno->telefone);
                printf("Ano de ingresso: %d\n", aluno->ano_ingresso);
                printf("Semestre de ingresso: %d\n", aluno->semestre_ingresso);
                printf("Data de nascimento: %s\n", aluno->data_nascimento);
                free(aluno);
            }
            else
            {
                printf("Aluno nao encontrado.\n");
            }
            pausar();
            break;
        }

        case 3:
        {
            // Atualizar aluno
            int matricula;
            printf("\n--- Atualizar Aluno ---\n");
            printf("Matricula do aluno: ");
            scanf("%d", &matricula);
            limpar_buffer();

            Aluno *aluno = aluno_buscar(aluno_mgr, matricula);
            if (aluno)
            {
                printf("\nDados atuais:\n");
                printf("Nome: %s\n", aluno->nome_aluno);
                printf("Endereco: %s\n", aluno->endereco);
                printf("Telefone: %s\n", aluno->telefone);

                printf("\nNovos dados:\n");
                printf("Nome: ");
                fgets(aluno->nome_aluno, sizeof(aluno->nome_aluno), stdin);
                remover_newline(aluno->nome_aluno);

                printf("Endereco: ");
                fgets(aluno->endereco, sizeof(aluno->endereco), stdin);
                remover_newline(aluno->endereco);

                printf("Telefone: ");
                fgets(aluno->telefone, sizeof(aluno->telefone), stdin);
                remover_newline(aluno->telefone);

                if (aluno_atualizar(aluno_mgr, aluno))
                {
                    printf("Aluno atualizado com sucesso!\n");
                }
                else
                {
                    printf("Erro ao atualizar aluno.\n");
                }
                free(aluno);
            }
            else
            {
                printf("Aluno nao encontrado.\n");
            }
            pausar();
            break;
        }

        case 4:
        {
            // Deletar aluno
            int matricula;
            printf("\n--- Deletar Aluno ---\n");
            printf("Matricula: ");
            scanf("%d", &matricula);
            limpar_buffer();

            printf("Confirma delecao do aluno %d? (S/N): ", matricula);
            char confirmacao;
            scanf("%c", &confirmacao);
            limpar_buffer();

            if (confirmacao == 'S' || confirmacao == 's')
            {
                if (aluno_deletar(aluno_mgr, matricula))
                {
                    printf("Aluno deletado com sucesso!\n");
                }
                else
                {
                    printf("Erro ao deletar aluno.\n");
                }
            }
            else
            {
                printf("Operacao cancelada.\n");
            }
            pausar();
            break;
        }

        case 5:
        {
            // Listar todos os alunos
            aluno_listar_todos(aluno_mgr);
            pausar();
            break;
        }

        case 0:
            break;

        default:
            printf("Opcao invalida!\n");
            pausar();
        }
    } while (opcao != 0);
}

// Menu de gerenciamento de disciplinas
void menu_disciplinas(DisciplinaManager *disciplina_mgr)
{
    int opcao;

    do
    {
        printf("\n");
        printf("===========================================\n");
        printf("      GERENCIAMENTO DE DISCIPLINAS\n");
        printf("===========================================\n");
        printf("1. Criar nova disciplina\n");
        printf("2. Buscar disciplina por codigo\n");
        printf("3. Atualizar dados da disciplina\n");
        printf("4. Deletar disciplina\n");
        printf("5. Listar todas as disciplinas\n");
        printf("0. Voltar\n");
        printf("===========================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1)
        {
            limpar_buffer();
            printf("Entrada invalida!\n");
            pausar();
            continue;
        }
        limpar_buffer();

        switch (opcao)
        {
        case 1:
        {
            // Criar nova disciplina
            Disciplina nova_disciplina;
            printf("\n--- Criar Nova Disciplina ---\n");
            printf("Codigo: ");
            fgets(nova_disciplina.codigo_disciplina, sizeof(nova_disciplina.codigo_disciplina), stdin);
            remover_newline(nova_disciplina.codigo_disciplina);

            printf("Nome: ");
            fgets(nova_disciplina.nome_disciplina, sizeof(nova_disciplina.nome_disciplina), stdin);
            remover_newline(nova_disciplina.nome_disciplina);

            if (disciplina_criar(disciplina_mgr, &nova_disciplina))
            {
                printf("Disciplina criada com sucesso!\n");
            }
            else
            {
                printf("Erro ao criar disciplina.\n");
            }
            pausar();
            break;
        }

        case 2:
        {
            // Buscar disciplina
            char codigo[20];
            printf("\n--- Buscar Disciplina ---\n");
            printf("Codigo: ");
            fgets(codigo, sizeof(codigo), stdin);
            remover_newline(codigo);

            Disciplina *disciplina = disciplina_buscar(disciplina_mgr, codigo);
            if (disciplina)
            {
                printf("\n--- Dados da Disciplina ---\n");
                printf("Codigo: %s\n", disciplina->codigo_disciplina);
                printf("Nome: %s\n", disciplina->nome_disciplina);
                free(disciplina);
            }
            else
            {
                printf("Disciplina nao encontrada.\n");
            }
            pausar();
            break;
        }

        case 3:
        {
            // Atualizar disciplina
            char codigo[20];
            printf("\n--- Atualizar Disciplina ---\n");
            printf("Codigo da disciplina: ");
            fgets(codigo, sizeof(codigo), stdin);
            remover_newline(codigo);

            Disciplina *disciplina = disciplina_buscar(disciplina_mgr, codigo);
            if (disciplina)
            {
                printf("\nDados atuais:\n");
                printf("Nome: %s\n", disciplina->nome_disciplina);

                printf("\nNovo nome: ");
                fgets(disciplina->nome_disciplina, sizeof(disciplina->nome_disciplina), stdin);
                remover_newline(disciplina->nome_disciplina);

                if (disciplina_atualizar(disciplina_mgr, disciplina))
                {
                    printf("Disciplina atualizada com sucesso!\n");
                }
                else
                {
                    printf("Erro ao atualizar disciplina.\n");
                }
                free(disciplina);
            }
            else
            {
                printf("Disciplina nao encontrada.\n");
            }
            pausar();
            break;
        }

        case 4:
        {
            // Deletar disciplina
            char codigo[20];
            printf("\n--- Deletar Disciplina ---\n");
            printf("Codigo: ");
            fgets(codigo, sizeof(codigo), stdin);
            remover_newline(codigo);

            printf("Confirma delecao da disciplina %s? (S/N): ", codigo);
            char confirmacao;
            scanf("%c", &confirmacao);
            limpar_buffer();

            if (confirmacao == 'S' || confirmacao == 's')
            {
                if (disciplina_deletar(disciplina_mgr, codigo))
                {
                    printf("Disciplina deletada com sucesso!\n");
                }
                else
                {
                    printf("Erro ao deletar disciplina.\n");
                }
            }
            else
            {
                printf("Operacao cancelada.\n");
            }
            pausar();
            break;
        }

        case 5:
        {
            // Listar todas as disciplinas
            disciplina_listar_todos(disciplina_mgr);
            pausar();
            break;
        }

        case 0:
            break;

        default:
            printf("Opcao invalida!\n");
            pausar();
        }
    } while (opcao != 0);
}

// Menu de gerenciamento de matrículas
void menu_matriculas(MatriculaManager *matricula_mgr)
{
    int opcao;

    do
    {
        printf("\n");
        printf("===========================================\n");
        printf("      GERENCIAMENTO DE MATRICULAS\n");
        printf("===========================================\n");
        printf("1. Criar nova matricula\n");
        printf("2. Buscar matricula por ID\n");
        printf("3. Atualizar media final\n");
        printf("4. Deletar matricula\n");
        printf("5. Listar todas as matriculas\n");
        printf("6. Listar matriculas por aluno\n");
        printf("7. Listar matriculas por disciplina\n");
        printf("0. Voltar\n");
        printf("===========================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1)
        {
            limpar_buffer();
            printf("Entrada invalida!\n");
            pausar();
            continue;
        }
        limpar_buffer();

        switch (opcao)
        {
        case 1:
        {
            // Criar nova matrícula
            Matricula nova_matricula;
            printf("\n--- Criar Nova Matricula ---\n");
            printf("ID da matricula: ");
            scanf("%d", &nova_matricula.id_matricula);
            limpar_buffer();

            printf("Matricula do aluno: ");
            scanf("%d", &nova_matricula.matricula_aluno);
            limpar_buffer();

            printf("Codigo da disciplina: ");
            fgets(nova_matricula.codigo_disciplina, sizeof(nova_matricula.codigo_disciplina), stdin);
            remover_newline(nova_matricula.codigo_disciplina);

            printf("Ano letivo: ");
            scanf("%d", &nova_matricula.ano_letivo);
            limpar_buffer();

            printf("Semestre letivo: ");
            scanf("%d", &nova_matricula.semestre_letivo);
            limpar_buffer();

            printf("Media final: ");
            scanf("%f", &nova_matricula.media_final);
            limpar_buffer();

            if (matricula_criar(matricula_mgr, &nova_matricula))
            {
                printf("Matricula criada com sucesso!\n");
            }
            else
            {
                printf("Erro ao criar matricula.\n");
            }
            pausar();
            break;
        }

        case 2:
        {
            // Buscar matrícula
            int id;
            printf("\n--- Buscar Matricula ---\n");
            printf("ID da matricula: ");
            scanf("%d", &id);
            limpar_buffer();

            Matricula *matricula = matricula_buscar(matricula_mgr, id);
            if (matricula)
            {
                printf("\n--- Dados da Matricula ---\n");
                printf("ID: %d\n", matricula->id_matricula);
                printf("Matricula do aluno: %d\n", matricula->matricula_aluno);
                printf("Codigo da disciplina: %s\n", matricula->codigo_disciplina);
                printf("Ano letivo: %d\n", matricula->ano_letivo);
                printf("Semestre letivo: %d\n", matricula->semestre_letivo);
                printf("Media final: %.2f\n", matricula->media_final);
                free(matricula);
            }
            else
            {
                printf("Matricula nao encontrada.\n");
            }
            pausar();
            break;
        }

        case 3:
        {
            // Atualizar média final
            int id;
            float nova_media;
            printf("\n--- Atualizar Media Final ---\n");
            printf("ID da matricula: ");
            scanf("%d", &id);
            limpar_buffer();

            printf("Nova media final: ");
            scanf("%f", &nova_media);
            limpar_buffer();

            if (matricula_atualizar_media(matricula_mgr, id, nova_media))
            {
                printf("Media atualizada com sucesso!\n");
            }
            else
            {
                printf("Erro ao atualizar media.\n");
            }
            pausar();
            break;
        }

        case 4:
        {
            // Deletar matrícula
            int id;
            printf("\n--- Deletar Matricula ---\n");
            printf("ID: ");
            scanf("%d", &id);
            limpar_buffer();

            printf("Confirma delecao da matricula %d? (S/N): ", id);
            char confirmacao;
            scanf("%c", &confirmacao);
            limpar_buffer();

            if (confirmacao == 'S' || confirmacao == 's')
            {
                if (matricula_deletar(matricula_mgr, id))
                {
                    printf("Matricula deletada com sucesso!\n");
                }
                else
                {
                    printf("Erro ao deletar matricula.\n");
                }
            }
            else
            {
                printf("Operacao cancelada.\n");
            }
            pausar();
            break;
        }

        case 5:
        {
            // Listar todas as matrículas
            matricula_listar_todos(matricula_mgr);
            pausar();
            break;
        }

        case 6:
        {
            // Listar matrículas por aluno
            int matricula_aluno;
            printf("Matricula do aluno: ");
            scanf("%d", &matricula_aluno);
            limpar_buffer();

            matricula_listar_por_aluno(matricula_mgr, matricula_aluno);
            pausar();
            break;
        }

        case 7:
        {
            // Listar matrículas por disciplina
            char codigo[20];
            printf("Codigo da disciplina: ");
            fgets(codigo, sizeof(codigo), stdin);
            remover_newline(codigo);

            matricula_listar_por_disciplina(matricula_mgr, codigo);
            pausar();
            break;
        }

        case 0:
            break;

        default:
            printf("Opcao invalida!\n");
            pausar();
        }
    } while (opcao != 0);
}

// Menu de consultas e relatórios
void menu_consultas(AlunoManager *aluno_mgr, DisciplinaManager *disciplina_mgr, MatriculaManager *matricula_mgr)
{
    int opcao;

    do
    {
        printf("\n");
        printf("===========================================\n");
        printf("       CONSULTAS E RELATORIOS\n");
        printf("===========================================\n");
        printf("1. Historico de um aluno\n");
        printf("2. Lista de alunos em uma disciplina\n");
        printf("3. Relatorio geral de matriculas\n");
        printf("0. Voltar\n");
        printf("===========================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1)
        {
            limpar_buffer();
            printf("Entrada invalida!\n");
            pausar();
            continue;
        }
        limpar_buffer();

        switch (opcao)
        {
        case 1:
        {
            // Histórico de um aluno
            int matricula_aluno;
            printf("\n--- Historico do Aluno ---\n");
            printf("Matricula do aluno: ");
            scanf("%d", &matricula_aluno);
            limpar_buffer();

            Aluno *aluno = aluno_buscar(aluno_mgr, matricula_aluno);
            if (aluno)
            {
                printf("\nAluno: %s (Matricula: %d)\n", aluno->nome_aluno, aluno->matricula);
                printf("Ingresso: %d/%d\n", aluno->ano_ingresso, aluno->semestre_ingresso);

                matricula_listar_historico_aluno(matricula_mgr, matricula_aluno, aluno_mgr, disciplina_mgr);
                free(aluno);
            }
            else
            {
                printf("Aluno nao encontrado.\n");
            }
            pausar();
            break;
        }

        case 2:
        {
            // Lista de alunos em uma disciplina
            char codigo[20];
            printf("\n--- Alunos em Disciplina ---\n");
            printf("Codigo da disciplina: ");
            fgets(codigo, sizeof(codigo), stdin);
            remover_newline(codigo);

            Disciplina *disciplina = disciplina_buscar(disciplina_mgr, codigo);
            if (disciplina)
            {
                printf("\nDisciplina: %s (%s)\n", disciplina->nome_disciplina, disciplina->codigo_disciplina);

                matricula_listar_alunos_em_disciplina(matricula_mgr, codigo, aluno_mgr, disciplina_mgr);
                free(disciplina);
            }
            else
            {
                printf("Disciplina nao encontrada.\n");
            }
            pausar();
            break;
        }

        case 3:
        {
            // Relatório geral
            printf("\n--- Relatorio Geral de Matriculas ---\n");

            int count_alunos = aluno_contar(aluno_mgr);
            int count_disciplinas = disciplina_contar(disciplina_mgr);
            int count_matriculas = matricula_contar(matricula_mgr);

            printf("\nTotal de alunos: %d\n", count_alunos);
            printf("Total de disciplinas: %d\n", count_disciplinas);
            printf("Total de matriculas: %d\n", count_matriculas);
            pausar();
            break;
        }

        case 0:
            break;

        default:
            printf("Opcao invalida!\n");
            pausar();
        }
    } while (opcao != 0);
}

// Função para importar dados dos CSVs
void importar_dados(AlunoManager *aluno_mgr, DisciplinaManager *disciplina_mgr, MatriculaManager *matricula_mgr)
{
    printf("\n===========================================\n");
    printf("       IMPORTACAO DE DADOS CSV\n");
    printf("===========================================\n");

    // Importar alunos
    printf("\nImportando alunos de 'aluno (1).csv'...\n");
    int alunos_importados = importar_alunos("aluno (1).csv", aluno_mgr);

    // Importar disciplinas
    printf("\nImportando disciplinas de 'disciplinas (1).csv'...\n");
    int disciplinas_importadas = importar_disciplinas("disciplinas (1).csv", disciplina_mgr);

    // Importar matrículas
    printf("\nImportando matriculas de 'matricula_aluno (1).csv'...\n");
    int matriculas_importadas = importar_matriculas("matricula_aluno (1).csv", matricula_mgr);

    printf("\n===========================================\n");
    printf("RESUMO DA IMPORTACAO:\n");
    printf("Alunos importados: %d\n", alunos_importados);
    printf("Disciplinas importadas: %d\n", disciplinas_importadas);
    printf("Matriculas importadas: %d\n", matriculas_importadas);
    printf("===========================================\n");

    pausar();
}

int main()
{
    AlunoManager *aluno_mgr = NULL;
    DisciplinaManager *disciplina_mgr = NULL;
    MatriculaManager *matricula_mgr = NULL;

    // Tentar abrir arquivos existentes ou criar novos
    printf("Inicializando sistema...\n");

    aluno_mgr = aluno_manager_open("alunos.dat", "alunos.idx");
    if (!aluno_mgr)
    {
        printf("Criando novos arquivos de alunos...\n");
        aluno_mgr = aluno_manager_create("alunos.dat", "alunos.idx");
    }

    disciplina_mgr = disciplina_manager_open("disciplinas.dat", "disciplinas.idx");
    if (!disciplina_mgr)
    {
        printf("Criando novos arquivos de disciplinas...\n");
        disciplina_mgr = disciplina_manager_create("disciplinas.dat", "disciplinas.idx");
    }

    matricula_mgr = matricula_manager_open("matriculas.dat", "matriculas.idx", aluno_mgr, disciplina_mgr);
    if (!matricula_mgr)
    {
        printf("Criando novos arquivos de matriculas...\n");
        matricula_mgr = matricula_manager_create("matriculas.dat", "matriculas.idx", aluno_mgr, disciplina_mgr);
    }

    if (!aluno_mgr || !disciplina_mgr || !matricula_mgr)
    {
        printf("Erro ao inicializar o sistema!\n");
        if (aluno_mgr)
            aluno_manager_close(aluno_mgr);
        if (disciplina_mgr)
            disciplina_manager_close(disciplina_mgr);
        if (matricula_mgr)
            matricula_manager_close(matricula_mgr);
        return 1;
    }

    printf("Sistema inicializado com sucesso!\n");

    // Importação automática se o sistema estiver vazio
    if (aluno_contar(aluno_mgr) == 0 &&
        disciplina_contar(disciplina_mgr) == 0 &&
        matricula_contar(matricula_mgr) == 0)
    {
        printf("\nSistema vazio detectado. Importando dados dos CSVs...\n");
        importar_dados(aluno_mgr, disciplina_mgr, matricula_mgr);
    }

    // Loop principal
    int opcao;
    do
    {
        exibir_menu_principal();

        if (scanf("%d", &opcao) != 1)
        {
            limpar_buffer();
            printf("Entrada invalida!\n");
            pausar();
            continue;
        }
        limpar_buffer();

        switch (opcao)
        {
        case 1:
            menu_alunos(aluno_mgr);
            break;
        case 2:
            menu_disciplinas(disciplina_mgr);
            break;
        case 3:
            menu_matriculas(matricula_mgr);
            break;
        case 4:
            menu_consultas(aluno_mgr, disciplina_mgr, matricula_mgr);
            break;
        case 0:
            printf("\nEncerrando sistema...\n");
            break;
        default:
            printf("Opcao invalida!\n");
            pausar();
        }
    } while (opcao != 0);

    // Limpeza
    matricula_manager_close(matricula_mgr);
    disciplina_manager_close(disciplina_mgr);
    aluno_manager_close(aluno_mgr);

    printf("Sistema encerrado com sucesso!\n");
    return 0;
}
