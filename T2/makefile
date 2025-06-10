# Flags e compiladores
CFLAGS  = -O3 -Wall -Wextra -march=native
LFLAGS  = -lm
PFLAGS  = -fopenmp
CC      = gcc
MPICC   = mpicc

# Diretórios e nomes
OUTPUT_DIR = weak-scalability

# Alvos padrão
all: seq par mpi

# Compila versão sequencial
seq: sequencial/lcs-seq.c 
	@echo "Compiling sequential version..."
	$(CC) $(CFLAGS) -o seq sequencial/lcs-seq.c $(LFLAGS)

# Compila versão MPI com antidiagonais
par: paralelo/lcs-parallel.c
	@echo "Compiling MPI antidiagonal version..."
	$(MPICC) $(CFLAGS) -o par paralelo/lcs-parallel.c $(LFLAGS)

# Alvo de execução (gera todos)
run: all

# Alvo de teste
test: all
	@echo "Running tests..."
	@mkdir -p $(OUTPUT_DIR)
	./executa_testes.sh
