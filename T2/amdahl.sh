#!/bin/bash
#SBATCH --job-name=amdahl_seq
#SBATCH --output=slurm_amdahl_%j.out
#SBATCH --error=slurm_amdahl_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=01:00:00

INPUT_DIR="./testes"
SIZE="150000"

FILE_A="${INPUT_DIR}/${SIZE}_A.in"
FILE_B="${INPUT_DIR}/${SIZE}_B.in"

echo "==============================="
echo "Calculando fração sequencial (Lei de Amdahl)"
echo "Tamanho da entrada: $SIZE"
echo "==============================="

# Executar o programa sequencial completo
TOTAL_TIME=$(./seq "$FILE_A" "$FILE_B" | grep "Time:" | awk '{print $2}')
echo "Tempo total (programa inteiro): $TOTAL_TIME segundos"

# Agora medir apenas o tempo da parte paralelizável (loop duplo LCS)
# Vamos compilar uma versão com marcação só no LCS
gcc -DMEASURE_LCS_ONLY -o seq_lcs_only seq.c

# Rodar essa versão "só LCS"
LCS_TIME=$(./seq_lcs_only "$FILE_A" "$FILE_B" | grep "LCS_Time:" | awk '{print $2}')
echo "Tempo dentro do loop LCS (parte paralelizável): $LCS_TIME segundos"

# Calcular fração sequencial: (Total - LCS) / Total
SEQUENTIAL_FRACTION=$(echo "scale=6; ($TOTAL_TIME - $LCS_TIME) / $TOTAL_TIME" | bc -l)
PARALLEL_FRACTION=$(echo "scale=6; $LCS_TIME / $TOTAL_TIME" | bc -l)

echo "==============================="
echo "Fração Sequencial estimada (1-f): $SEQUENTIAL_FRACTION"
echo "Fração Paralelizável estimada (f): $PARALLEL_FRACTION"
echo "==============================="

# Salvar no arquivo
echo "Entrada: $SIZE" > amdahl_fraction.txt
echo "Tempo total: $TOTAL_TIME segundos" >> amdahl_fraction.txt
echo "Tempo paralelizável (LCS): $LCS_TIME segundos" >> amdahl_fraction.txt
echo "Fração sequencial (1-f): $SEQUENTIAL_FRACTION" >> amdahl_fraction.txt
echo "Fração paralelizável (f): $PARALLEL_FRACTION" >> amdahl_fraction.txt
