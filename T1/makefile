CFLAGS = -O3 -Wall -Wextra -march=native -funroll-loops -ffast-math
LFLAGS = -lm
PFLAGS = -fopenmp
CC = gcc

# variables
OUTPUT_DIR = weak-scalability

all: seq par

seq: sequencial/lcs-seq.c 
	@echo "Compiling sequential version..."
	$(CC) $(CFLAGS) -o seq sequencial/lcs-seq.c $(LFLAGS)

par: paralelo/lcs-parallel.c
	@echo "Compiling parallel version..."
	$(CC) $(CFLAGS) $(PFLAGS) -o par paralelo/lcs-parallel.c $(LFLAGS)

run: all


test: all
	@echo "Running tests..."
	@mkdir -p $(OUTPUT_DIR)
	./executa_testes.sh
