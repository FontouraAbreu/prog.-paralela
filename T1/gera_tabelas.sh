#!/bin/bash

RESULTS_DIR="./output"

generate_speedup_csv() {
    OUTPUT_CSV="speedup.csv"
    echo -n "Entry" > "$OUTPUT_CSV"
    for exp in 1 2 3 4; do
        t=$((2 ** exp))
        echo -n ",$t" >> "$OUTPUT_CSV"
    done
    echo "" >> "$OUTPUT_CSV"

    for seqfile in "$RESULTS_DIR"/seq_result_*.txt; do
        entry=$(basename "$seqfile" | sed 's/seq_result_//' | sed 's/.txt//')
        parfile="$RESULTS_DIR/par_result_${entry}.txt"
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
    echo -n "Entry,sequential,sequential-dev" > "$OUTPUT_CSV"
    for exp in 1 2 3 4; do
        t=$((2 ** exp))
        echo -n ",$t-avg,$t-dev" >> "$OUTPUT_CSV"
    done
    echo "" >> "$OUTPUT_CSV"

    for seqfile in "$RESULTS_DIR"/seq_result_*.txt; do
        entry=$(basename "$seqfile" | sed 's/seq_result_//' | sed 's/.txt//')
        parfile="$RESULTS_DIR/par_result_${entry}.txt"
        if [[ ! -f "$parfile" ]]; then
            echo "Missing parallel result for $entry"
            continue
        fi

        seq_time=$(grep "AVG Time" "$seqfile" | awk '{print $3}')
        seq_dev=$(grep "Deviation" "$seqfile" | awk '{print $2}')
        echo -n "$entry,$seq_time,$seq_dev" >> "$OUTPUT_CSV"

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

generate_strong_efficiency_csv() {
    OUTPUT_CSV="strong_efficiency.csv"
    echo "Threads,AVG Time,Efficiency" > "$OUTPUT_CSV"

    # Agora lê o tempo sequencial real do arquivo seq_result_180000.txt
    seq_time=$(grep "AVG Time" "$RESULTS_DIR/seq_result_180000.txt" | awk '{print $3}')
    par_file="$RESULTS_DIR/par_result_180000.txt"

    for threads in 2 4 8 16; do
        avg_time=$(awk -v threads=$threads '
            $0 ~ "Threads: "threads {
                getline; print $3
            }
        ' "$par_file")

        if [[ -z "$avg_time" ]]; then
            echo "$threads,NA,NA" >> "$OUTPUT_CSV"
        else
            efficiency=$(awk -v seq="$seq_time" -v par="$avg_time" -v p="$threads" 'BEGIN { printf "%.4f", seq / (par * p) }')
            echo "$threads,$avg_time,$efficiency" >> "$OUTPUT_CSV"
        fi
    done

    echo "Strong Efficiency CSV generated: $OUTPUT_CSV"
}

generate_weak_efficiency_csv() {
    OUTPUT_CSV="weak_efficiency.csv"
    echo "Threads,Size,AVG Time,Efficiency" > "$OUTPUT_CSV"

    weak_file="$RESULTS_DIR/weak_scaling_results.txt"

    # Base da weak scaling é o tempo de 2 threads
    base_time=$(awk '
        $0 ~ "Threads: 2" {
            getline; getline;
            print $3
        }
    ' "$weak_file")

    for threads in 2 4 8 16; do
        size=$(awk -v threads=$threads '
            $0 ~ "Threads: "threads {
                getline;
                print $2
            }
        ' "$weak_file")

        avg_time=$(awk -v threads=$threads '
            $0 ~ "Threads: "threads {
                getline; getline;
                print $3
            }
        ' "$weak_file")

        if [[ -z "$avg_time" || -z "$size" ]]; then
            echo "$threads,NA,NA,NA" >> "$OUTPUT_CSV"
        else
            efficiency=$(awk -v base="$base_time" -v par="$avg_time" 'BEGIN { printf "%.4f", base / par }')
            echo "$threads,$size,$avg_time,$efficiency" >> "$OUTPUT_CSV"
        fi
    done

    echo "Weak Efficiency CSV generated: $OUTPUT_CSV"
}

# Executa as gerações
generate_speedup_csv
generate_time_csv
generate_strong_efficiency_csv
generate_weak_efficiency_csv
