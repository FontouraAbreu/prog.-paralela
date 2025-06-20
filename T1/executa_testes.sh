#!/bin/bash

TEST_DIR="./testes"
SEQ_EXEC="./seq"
PAR_EXEC="./par"
OUTPUT="./output"
REPEATS=2

THREAD_STEPS=(2 4 8 16)
ENTRY_SIZES=(20000 30000 40000 50000 60000 70000 80000 90000 100000 110000 120000 130000 140000 150000 160000 170000 180000 190000 200000)

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

mkdir -p "$OUTPUT"

# ===== Loop original: Variação de tamanhos de entrada para cada número de threads =====

for entry_size in "${ENTRY_SIZES[@]}"; do
    fileA="${TEST_DIR}/${entry_size}_A.in"
    fileB="${TEST_DIR}/${entry_size}_B.in"

    if [[ ! -f "$fileA" || ! -f "$fileB" ]]; then
        echo "Missing files for entry size $entry_size, skipping..."
        continue
    fi

    echo "Running tests for entry size: $entry_size"

    # Arquivos temporários
    tmp_seq="${OUTPUT}/tmp_${entry_size}_seq.txt"
    declare -A tmp_par
    for threads in "${THREAD_STEPS[@]}"; do
        tmp_par[$threads]="${OUTPUT}/tmp_${entry_size}_${threads}.txt"
        rm -f "${tmp_par[$threads]}"
    done
    rm -f "$tmp_seq"

    # LOOP intercalando execuções
    for ((repeat = 0; repeat < REPEATS; repeat++)); do
        # Sequencial
        { /usr/bin/time -f "%e" "$SEQ_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$tmp_seq"

        # Paralelos
        for threads in "${THREAD_STEPS[@]}"; do
            export OMP_NUM_THREADS=$threads
            { /usr/bin/time -f "%e" "$PAR_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "${tmp_par[$threads]}"
        done
    done

    # Saída Sequencial
    seq_output="${OUTPUT}/seq_result_${entry_size}.txt"
    echo -n > "$seq_output"
    echo "Execution: Sequential" >> "$seq_output"
    compute_stats < "$tmp_seq" >> "$seq_output"

    # Saída Paralela
    par_output="${OUTPUT}/par_result_${entry_size}.txt"
    echo -n > "$par_output"
    for threads in "${THREAD_STEPS[@]}"; do
        echo "Threads: $threads" >> "$par_output"
        compute_stats < "${tmp_par[$threads]}" >> "$par_output"
        echo "" >> "$par_output"
    done

    echo "Results saved to $seq_output and $par_output"

    # Limpa temporários
    rm -f "$tmp_seq"
    for threads in "${THREAD_STEPS[@]}"; do
        rm -f "${tmp_par[$threads]}"
    done
done

# ===== Novo loop: Escalabilidade Fraca (Weak Scaling) =====

echo "Running Weak Scaling tests..."

# Tamanhos de entrada para weak scaling
declare -A WEAK_SIZES
WEAK_SIZES[2]=20000
WEAK_SIZES[4]=40000
WEAK_SIZES[8]=80000
WEAK_SIZES[16]=160000

weak_result_file="${OUTPUT}/weak_scaling_results.txt"
rm -f "$weak_result_file"

for threads in "${THREAD_STEPS[@]}"; do
    entry_size=${WEAK_SIZES[$threads]}
    fileA="${TEST_DIR}/${entry_size}_A.in"
    fileB="${TEST_DIR}/${entry_size}_B.in"

    if [[ ! -f "$fileA" || ! -f "$fileB" ]]; then
        echo "Missing files for weak scaling size $entry_size, skipping..."
        continue
    fi

    tmp_file="${OUTPUT}/tmp_weak_${threads}.txt"
    rm -f "$tmp_file"

    for ((repeat = 0; repeat < REPEATS; repeat++)); do
        export OMP_NUM_THREADS=$threads
        { /usr/bin/time -f "%e" "$PAR_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$tmp_file"
    done

    echo "Threads: $threads" >> "$weak_result_file"
    echo "Size: $entry_size" >> "$weak_result_file"
    compute_stats < "$tmp_file" >> "$weak_result_file"
    echo "" >> "$weak_result_file"

    rm -f "$tmp_file"
done

echo "Weak scaling results saved to $weak_result_file"
