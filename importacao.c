#include "importacao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Função auxiliar para remover caracteres de nova linha
void remover_newline(char *str)
{
    if (!str)
        return;

    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
        str[len - 1] = '\0';
        len--;
    }
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
        str[len - 1] = '\0';
    }
}

// Função auxiliar para extrair campo de CSV (lida com vírgulas e aspas)
char *extrair_campo_csv(char **linha)
{
    if (!linha || !*linha)
        return NULL;

    char *start = *linha;
    char *end;

    // Pular espaços iniciais
    while (*start == ' ')
        start++;

    // Se o campo começa com aspas
    if (*start == '"')
    {
        start++; // Pula a aspa inicial
        end = start;
        int found_quote = 0;

        // Procura pela aspa de fechamento
        while (*end && *end != '"')
        {
            end++;
        }

        if (*end == '"')
        {
            found_quote = 1;
            *end = '\0';
            end++;
        }

        // Se encontramos a aspa, pular espaços e vírgula
        if (found_quote)
        {
            // Pula espaços em branco
            while (*end == ' ' || *end == '\t')
                end++;
            if (*end == ',')
                end++;
        }
    }
    else
    {
        // Campo sem aspas - procura pela vírgula
        end = start;
        while (*end && *end != ',' && *end != '\n' && *end != '\r')
        {
            end++;
        }

        if (*end == ',')
        {
            *end = '\0';
            end++;
        }
        else if (*end)
        {
            *end = '\0';
        }
    }

    *linha = end;
    return start;
}

// Função auxiliar para gerar média aleatória entre 0.0 e 10.0
float gerar_media_aleatoria()
{
    return ((float)rand() / RAND_MAX) * 10.0f;
}

// Função para importar alunos de um arquivo CSV
int importar_alunos(const char *csv_filename, AlunoManager *manager)
{
    if (!csv_filename || !manager)
    {
        printf("Erro: Parâmetros inválidos para importar_alunos.\n");
        return 0;
    }

    FILE *file = fopen(csv_filename, "r");
    if (!file)
    {
        printf("Erro: Não foi possível abrir o arquivo %s\n", csv_filename);
        return 0;
    }

    char linha[1024];
    int count = 0;
    int linha_num = 0;

    // Pula o cabeçalho (primeira linha)
    if (fgets(linha, sizeof(linha), file))
    {
        linha_num++;
    }

    // Lê cada linha do arquivo
    while (fgets(linha, sizeof(linha), file))
    {
        linha_num++;
        remover_newline(linha);

        // Debug print
        printf("Processando aluno linha %d: %s\n", linha_num, linha);

        // Ignora linhas vazias
        if (strlen(linha) == 0)
            continue;

        char *ptr = linha;
        Aluno aluno;

        // Extrai campos: matricula,nome_aluno,endereco,telefone,ano_ingresso,semestre_ingresso,data_nascimento
        char *campo;

        // matricula
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo matricula ausente, pulando.\n", linha_num);
            continue;
        }
        aluno.matricula = atoi(campo);

        // nome_aluno
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo nome_aluno ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(aluno.nome_aluno, campo, sizeof(aluno.nome_aluno) - 1);
        aluno.nome_aluno[sizeof(aluno.nome_aluno) - 1] = '\0';

        // endereco
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo endereco ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(aluno.endereco, campo, sizeof(aluno.endereco) - 1);
        aluno.endereco[sizeof(aluno.endereco) - 1] = '\0';

        // telefone
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo telefone ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(aluno.telefone, campo, sizeof(aluno.telefone) - 1);
        aluno.telefone[sizeof(aluno.telefone) - 1] = '\0';

        // ano_ingresso
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo ano_ingresso ausente, pulando.\n", linha_num);
            continue;
        }
        aluno.ano_ingresso = atoi(campo);

        // semestre_ingresso
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo semestre_ingresso ausente, pulando.\n", linha_num);
            continue;
        }
        aluno.semestre_ingresso = atoi(campo);

        // data_nascimento
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo data_nascimento ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(aluno.data_nascimento, campo, sizeof(aluno.data_nascimento) - 1);
        aluno.data_nascimento[sizeof(aluno.data_nascimento) - 1] = '\0';

        // Tenta criar o aluno
        if (aluno_criar(manager, &aluno))
        {
            count++;
        }
        else
        {
            printf("Aviso: Linha %d - não foi possível criar aluno com matrícula %d\n",
                   linha_num, aluno.matricula);
        }
    }

    fclose(file);
    printf("Importação de alunos concluída: %d registros importados.\n", count);
    return count;
}

// Função para importar disciplinas de um arquivo CSV
int importar_disciplinas(const char *csv_filename, DisciplinaManager *manager)
{
    if (!csv_filename || !manager)
    {
        printf("Erro: Parâmetros inválidos para importar_disciplinas.\n");
        return 0;
    }

    FILE *file = fopen(csv_filename, "r");
    if (!file)
    {
        printf("Erro: Não foi possível abrir o arquivo %s\n", csv_filename);
        return 0;
    }

    char linha[1024];
    int count = 0;
    int linha_num = 0;

    // Pula o cabeçalho (primeira linha)
    if (fgets(linha, sizeof(linha), file))
    {
        linha_num++;
    }

    // Lê cada linha do arquivo
    while (fgets(linha, sizeof(linha), file))
    {
        linha_num++;
        remover_newline(linha);

        // Debug print
        printf("Processando disciplina linha %d: %s\n", linha_num, linha);

        // Ignora linhas vazias
        if (strlen(linha) == 0)
            continue;

        char *ptr = linha;
        Disciplina disciplina;

        // Extrai campos: codigo_disciplina,nome_disciplina
        char *campo;

        // codigo_disciplina
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo codigo_disciplina ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(disciplina.codigo_disciplina, campo, sizeof(disciplina.codigo_disciplina) - 1);
        disciplina.codigo_disciplina[sizeof(disciplina.codigo_disciplina) - 1] = '\0';

        // nome_disciplina
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo nome_disciplina ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(disciplina.nome_disciplina, campo, sizeof(disciplina.nome_disciplina) - 1);
        disciplina.nome_disciplina[sizeof(disciplina.nome_disciplina) - 1] = '\0';

        // Tenta criar a disciplina
        if (disciplina_criar(manager, &disciplina))
        {
            count++;
        }
        else
        {
            printf("Aviso: Linha %d - não foi possível criar disciplina com código %s\n",
                   linha_num, disciplina.codigo_disciplina);
        }
    }

    fclose(file);
    printf("Importação de disciplinas concluída: %d registros importados.\n", count);
    return count;
}

// Função para importar matrículas de um arquivo CSV
int importar_matriculas(const char *csv_filename, MatriculaManager *manager)
{
    if (!csv_filename || !manager)
    {
        printf("Erro: Parâmetros inválidos para importar_matriculas.\n");
        return 0;
    }

    FILE *file = fopen(csv_filename, "r");
    if (!file)
    {
        printf("Erro: Não foi possível abrir o arquivo %s\n", csv_filename);
        return 0;
    }

    // Inicializa gerador de números aleatórios
    srand(time(NULL));

    char linha[1024];
    int count = 0;
    int linha_num = 0;

    // Pula o cabeçalho (primeira linha)
    if (fgets(linha, sizeof(linha), file))
    {
        linha_num++;
    }

    // Lê cada linha do arquivo
    while (fgets(linha, sizeof(linha), file))
    {
        linha_num++;
        remover_newline(linha);

        // Debug print
        printf("Processando matrícula linha %d: %s\n", linha_num, linha);

        // Ignora linhas vazias
        if (strlen(linha) == 0)
            continue;

        char *ptr = linha;
        Matricula matricula;

        // Extrai campos: id_matricula,matricula_aluno,codigo_disciplina,ano_letivo,semestre_letivo,media_final
        char *campo;

        // id_matricula
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo id_matricula ausente, pulando.\n", linha_num);
            continue;
        }
        matricula.id_matricula = atoi(campo);

        // matricula_aluno
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo matricula_aluno ausente, pulando.\n", linha_num);
            continue;
        }
        matricula.matricula_aluno = atoi(campo);

        // codigo_disciplina
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo codigo_disciplina ausente, pulando.\n", linha_num);
            continue;
        }
        strncpy(matricula.codigo_disciplina, campo, sizeof(matricula.codigo_disciplina) - 1);
        matricula.codigo_disciplina[sizeof(matricula.codigo_disciplina) - 1] = '\0';

        // ano_letivo
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo ano_letivo ausente, pulando.\n", linha_num);
            continue;
        }
        matricula.ano_letivo = atoi(campo);

        // semestre_letivo
        campo = extrair_campo_csv(&ptr);
        if (!campo)
        {
            printf("Aviso: Linha %d - campo semestre_letivo ausente, pulando.\n", linha_num);
            continue;
        }
        matricula.semestre_letivo = atoi(campo);

        // media_final (pode estar vazio)
        campo = extrair_campo_csv(&ptr);
        if (!campo || strlen(campo) == 0)
        {
            // Gera valor aleatório para média ausente
            matricula.media_final = gerar_media_aleatoria();
            printf("Info: Linha %d - média final gerada aleatoriamente: %.2f\n",
                   linha_num, matricula.media_final);
        }
        else
        {
            matricula.media_final = atof(campo);
        }

        // Tenta criar a matrícula
        if (matricula_criar(manager, &matricula))
        {
            count++;
        }
        else
        {
            printf("Aviso: Linha %d - não foi possível criar matrícula com ID %d\n",
                   linha_num, matricula.id_matricula);
        }
    }

    fclose(file);
    printf("Importação de matrículas concluída: %d registros importados.\n", count);
    return count;
}
