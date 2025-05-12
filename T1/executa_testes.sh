#!/bin/bash

TEST_DIR="./testes"
SEQ_EXEC="./seq"
PAR_EXEC="./par"
OUTPUT="./output"
REPEATS=20
THREAD_STEPS=(2 4 8 16)


# Function to compute average and std deviation using awk
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


for fileA in "$TEST_DIR"/*_A.in; do
    base=$(basename "$fileA" _A.in)
    fileB="$TEST_DIR/${base}_B.in"

    if [[ ! -f "$fileB" ]]; then
        echo "Missing pair for $fileA, skipping."
        continue
    fi

    echo "Running test: $base"

    # Sequential
    echo -n > "${OUTPUT}/sequential_result_${base}.txt"
    for ((i = 0; i < REPEATS; i++)); do
        { /usr/bin/time -f "%e" "$SEQ_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> tmp_seq_times.txt
    done
    compute_stats < tmp_seq_times.txt >> "${OUTPUT}/sequential_result_${base}.txt"
    rm -f tmp_seq_times.txt

    # Parallel
    echo -n > "${OUTPUT}/parallel_result_${base}.txt"
    for threads in "${THREAD_STEPS[@]}"; do
        export OMP_NUM_THREADS=$threads
        echo "Threads: $threads" >> "${OUTPUT}/parallel_result_${base}.txt"
        for ((i = 0; i < REPEATS; i++)); do
            { /usr/bin/time -f "%e" "$PAR_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> tmp_par_times.txt
        done
        compute_stats < tmp_par_times.txt >> "${OUTPUT}/parallel_result_${base}.txt"
        echo "" >> "${OUTPUT}/parallel_result_${base}.txt"
        rm -f tmp_par_times.txt
    done
done
