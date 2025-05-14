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

# Ensure output directory exists
mkdir -p "$OUTPUT"

for fileA in "$TEST_DIR"/*_A.in; do
    base=$(basename "$fileA" _A.in)
    fileB="$TEST_DIR/${base}_B.in"

    if [[ ! -f "$fileB" ]]; then
        echo "Missing pair for $fileA, skipping."
        continue
    fi

    echo "Running strong scalability test: $base"

    # Clear temporary files
    seq_tmp="${OUTPUT}/tmp_seq_times_${base}.txt"
    rm -f "$seq_tmp"
    declare -A par_tmp_files
    for threads in "${THREAD_STEPS[@]}"; do
        par_tmp_files[$threads]="${OUTPUT}/tmp_par_times_${base}_${threads}.txt"
        rm -f "${par_tmp_files[$threads]}"
    done

    for ((i = 0; i < REPEATS; i++)); do
        # Sequential run
        { /usr/bin/time -f "%e" "$SEQ_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$seq_tmp"

        # Parallel runs for all thread counts
        for threads in "${THREAD_STEPS[@]}"; do
            export OMP_NUM_THREADS=$threads
            { /usr/bin/time -f "%e" "$PAR_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "${par_tmp_files[$threads]}"
        done
    done

    # Output sequential results
    seq_out="${OUTPUT}/sequential_result_${base}.txt"
    compute_stats < "$seq_tmp" > "$seq_out"
    rm -f "$seq_tmp"

    # Output parallel results
    par_out="${OUTPUT}/parallel_result_${base}.txt"
    echo -n > "$par_out"
    for threads in "${THREAD_STEPS[@]}"; do
        echo "Threads: $threads" >> "$par_out"
        compute_stats < "${par_tmp_files[$threads]}" >> "$par_out"
        echo "" >> "$par_out"
        rm -f "${par_tmp_files[$threads]}"
    done

done

