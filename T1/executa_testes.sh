#!/bin/bash

TEST_DIR="./testes"
SEQ_EXEC="./seq"
PAR_EXEC="./par"
OUTPUT="./weak-scalability"
REPEATS=10

# Entry sizes for weak scalability: one per thread count
ENTRY_SIZES=(20000 40000 80000 160000)
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

# Loop through each thread step and corresponding entry size
for i in "${!THREAD_STEPS[@]}"; do
    threads=${THREAD_STEPS[$i]}
    entry_size=${ENTRY_SIZES[$i]}

    fileA="${TEST_DIR}/${entry_size}_A.in"
    fileB="${TEST_DIR}/${entry_size}_B.in"

    if [[ ! -f "$fileA" || ! -f "$fileB" ]]; then
        echo "Missing files for entry size $entry_size, skipping..."
        continue
    fi

    echo "Running weak scalability test for $threads threads with entry size $entry_size..."

    tmp_file="${OUTPUT}/tmp_weak_${threads}.txt"
    rm -f "$tmp_file"

    for ((j = 0; j < REPEATS; j++)); do
        if [[ $threads -eq 1 ]]; then
            { /usr/bin/time -f "%e" "$SEQ_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$tmp_file"
        else
            export OMP_NUM_THREADS=$threads
            { /usr/bin/time -f "%e" "$PAR_EXEC" "$fileA" "$fileB" > /dev/null; } 2>> "$tmp_file"
        fi
    done

    # Output result
    result_file="${OUTPUT}/weak_result_${threads}_threads.txt"
    compute_stats < "$tmp_file" > "$result_file"
    echo "Results saved to $result_file"
    rm -f "$tmp_file"
done
