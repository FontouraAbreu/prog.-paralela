# Flags e compiladores
CFLAGS  = -O3 -Wall -Wextra -march=native
LFLAGS  = -lm
PFLAGS  = -fopenmp
CC      = gcc
MPICC   = mpicc

# Diretórios e nomes
OUTPUT_DIR = weak-scalability

# Alvos padrão
all: seq par

# Compila versão sequencial
seq: sequencial/lcs-seq.c 
	$(CC) $(CFLAGS) -o seq sequencial/lcs-seq.c $(LFLAGS)

# Compila versão MPI com antidiagonais
par: paralelo/lcs-parallel.c
	$(MPICC) $(CFLAGS) -o par paralelo/lcs-parallel.c $(LFLAGS)

# Alvo de execução (gera todos)
run: all

# Alvo de teste
test: all
	@mkdir -p $(OUTPUT_DIR)
	./executa_testes.sh

clean:
	rm -f seq par