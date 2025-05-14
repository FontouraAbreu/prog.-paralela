#!/bin/bash

RESULTS_DIR="./output"
MODE="$2"

generate_speedup_csv() {
    OUTPUT_CSV="speedup.csv"
    echo -n "Entry" > "$OUTPUT_CSV"
    for exp in 1 2 3 4; do
        t=$((2 ** exp))
        echo -n ",$t" >> "$OUTPUT_CSV"
    done
    echo "" >> "$OUTPUT_CSV"

    for seqfile in "$RESULTS_DIR"/sequential_result_*.txt; do
        entry=$(basename "$seqfile" | sed 's/sequential_result_//' | sed 's/.txt//')
        parfile="$RESULTS_DIR/parallel_result_${entry}.txt"
        if [[ ! -f "$parfile" ]]; then
            echo "Missing parallel result for $entry"
            continue
        fi

        seq_time=$(grep "AVG Time" "$seqfile" | awk '{print $3}')
        echo -n "$entry,$seq_time" > temp_speed.csv

        for exp in 1 2 3 4; do
            t=$((2 ** exp))
            avg_time=$(awk -v threads=$t '
                $0 ~ "Threads: "threads {
                    getline; print $3
                }
            ' "$parfile")

            if [[ -z "$avg_time" ]]; then
                echo -n ",NA" >> temp_speed.csv
            else
                speedup=$(awk -v seq="$seq_time" -v par="$avg_time" 'BEGIN { printf "%.4f", seq/par }')
                echo -n ",$speedup" >> temp_speed.csv
            fi
        done
        echo "" >> temp_speed.csv
        cut -d',' -f1,3- temp_speed.csv >> "$OUTPUT_CSV"
        rm temp_speed.csv
    done

    echo "Speedup CSV generated: $OUTPUT_CSV"
}

generate_time_csv() {
    OUTPUT_CSV="time.csv"
    echo -n "Entry,sequential" > "$OUTPUT_CSV"
    for exp in 1 2 3 4; do
        t=$((2 ** exp))
        echo -n ",$t-avg,$t-dev" >> "$OUTPUT_CSV"
    done
    echo "" >> "$OUTPUT_CSV"

    for seqfile in "$RESULTS_DIR"/sequential_result_*.txt; do
        entry=$(basename "$seqfile" | sed 's/sequential_result_//' | sed 's/.txt//')
        parfile="$RESULTS_DIR/parallel_result_${entry}.txt"
        if [[ ! -f "$parfile" ]]; then
            echo "Missing parallel result for $entry"
            continue
        fi

        seq_time=$(grep "AVG Time" "$seqfile" | awk '{print $3}')
        echo -n "$entry,$seq_time" >> "$OUTPUT_CSV"

        for exp in 1 2 3 4; do
            t=$((2 ** exp))
            avg=$(awk -v threads=$t '
                $0 ~ "Threads: "threads {
                    getline; print $3
                }
            ' "$parfile")
            dev=$(awk -v threads=$t '
                $0 ~ "Threads: "threads {
                    getline; getline; print $2
                }
            ' "$parfile")

            if [[ -z "$avg" || -z "$dev" ]]; then
                echo -n ",NA,NA" >> "$OUTPUT_CSV"
            else
                echo -n ",$avg,$dev" >> "$OUTPUT_CSV"
            fi
        done

        echo "" >> "$OUTPUT_CSV"
    done

    echo "Time CSV generated: $OUTPUT_CSV"
}

# Choose based on mode
generate_speedup_csv
generate_time_csv

