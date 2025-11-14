# Makefile para Sistema de Gerenciamento Acadêmico
# Compila todos os módulos e gera o executável

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
LDFLAGS = -lm

# Nome do executável
TARGET = sga

# Arquivos fonte
SOURCES = btree.c aluno.c disciplina.c matricula.c importacao.c main.c

# Arquivos objeto (gerados a partir dos .c)
OBJECTS = $(SOURCES:.c=.o)

# Arquivos de cabeçalho
HEADERS = btree.h aluno.h disciplina.h matricula.h importacao.h

# Regra padrão: compilar tudo
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Compilação concluída! Executável: $(TARGET)"

# Regra para compilar arquivos .c em .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Dependências específicas
btree.o: btree.c btree.h
aluno.o: aluno.c aluno.h btree.h
disciplina.o: disciplina.c disciplina.h btree.h
matricula.o: matricula.c matricula.h btree.h aluno.h disciplina.h
importacao.o: importacao.c importacao.h aluno.h disciplina.h matricula.h
main.o: main.c aluno.h disciplina.h matricula.h importacao.h btree.h

# Limpar arquivos compilados
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.dat *.idx
	@echo "Arquivos compilados e de dados removidos"

# Recompilar tudo do zero
rebuild: clean all

# Executar o programa após compilar
run: $(TARGET)
	./$(TARGET)

# Compilar e executar
go: all run

# Ajuda
help:
	@echo "Alvos disponíveis:"
	@echo "  all      - Compila o projeto (padrão)"
	@echo "  clean    - Remove arquivos compilados e de dados"
	@echo "  rebuild  - Limpa e recompila tudo"
	@echo "  run      - Executa o programa"
	@echo "  go       - Compila e executa"
	@echo "  help     - Mostra esta mensagem"

.PHONY: all clean rebuild run go help
