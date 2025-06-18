#!/bin/bash

TEST_DIR="./testes"
SEQ_EXEC="./seq"
PAR_EXEC="./par"
REPEATS=2

ENTRY_SIZES=(20000 30000 40000 50000 60000 70000 80000 90000 100000 110000 120000 130000 140000 150000 160000)

# Função para calcular média e desvio padrão
compute_stats() {
    awk '
    {
        sum += $1;
        sumsq += ($1)^2;
        n++;
    }
    END {
        avg = sum / n;
        std = sqrt(sumsq / n - avg^2);
        printf "AVG Time: %.6f seconds\nDeviation: %.6f\n", avg, std;
    }'
}

threads=${THREAD_STEPS[0]}  # Valor exportado por cada slurm_Xtasks.sh

# Definindo diretório de saída específico por configuração de processos
OUTPUT="./output/${threads}_tasks"
mkdir -p "$OUTPUT"

for entry_size in "${ENTRY_SIZES[@]}"; do
    fileA="${TEST_DIR}/${entry_size}_A.in"
    fileB="${TEST_DIR}/${entry_size}_B.in"

    if [[ ! -f "$fileA" || ! -f "$fileB" ]]; then
        echo "Missing files for entry size $entry_size, skipping..."
        continue
    fi

    echo "Running tests for entry size: $entry_size with $threads MPI processes"

    tmp_seq="${OUTPUT}/tmp_${entry_size}_seq.txt"
    tmp_par="${OUTPUT}/tmp_${entry_size}_${threads}.txt"
    rm -f "$tmp_seq" "$tmp_par"

    for ((repeat = 0; repeat < REPEATS; repeat++)); do
        # Sequencial
        { /usr/bin/time -f "%e" "$SEQ_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$tmp_seq"

        # Paralelo (MPI via SLURM srun com número fixo de tasks)
        mem_per_cpu=$(( (entry_size / 10000 + 1) * 10 ))
        { /usr/bin/time -f "%e" srun --ntasks=$threads --mem-per-cpu=${mem_per_cpu}M --nodes=1 "$PAR_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$tmp_par"
    done

    # Sequencial Results
    seq_output="${OUTPUT}/seq_result_${entry_size}.txt"
    echo "Execution: Sequential" > "$seq_output"
    compute_stats < "$tmp_seq" >> "$seq_output"

    # Paralelo Results
    par_output="${OUTPUT}/par_result_${entry_size}_${threads}.txt"
    echo "Processes: $threads" > "$par_output"
    compute_stats < "$tmp_par" >> "$par_output"

    echo "Results saved to $seq_output and $par_output"

    rm -f "$tmp_seq" "$tmp_par"
done
