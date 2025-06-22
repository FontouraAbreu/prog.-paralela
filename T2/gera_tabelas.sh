#!/bin/bash

RESULTS_DIR="./output"

THREAD_COUNTS=(2 4 8 12)

generate_speedup_csv() {
    OUTPUT_CSV="speedup.csv"
    echo -n "Entry" > "$OUTPUT_CSV"
    for t in "${THREAD_COUNTS[@]}"; do
        echo -n ",$t" >> "$OUTPUT_CSV"
    done
    echo "" >> "$OUTPUT_CSV"

    for seqfile in "$RESULTS_DIR"/summary_seq_*.txt; do
        entry=$(basename "$seqfile" | sed 's/summary_seq_//' | sed 's/.txt//')
        parfile="$RESULTS_DIR/summary_par_${entry}.txt"
        if [[ ! -f "$parfile" ]]; then
            echo "Missing parallel result for $entry"
            continue
        fi

        seq_time=$(grep "AVG Time" "$seqfile" | awk '{print $3}')
        echo -n "$entry" > temp_speed.csv

        for t in "${THREAD_COUNTS[@]}"; do
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
        cut -d',' -f1- temp_speed.csv >> "$OUTPUT_CSV"
        rm temp_speed.csv
    done

    echo "Speedup CSV generated: $OUTPUT_CSV"
}

generate_time_csv() {
    OUTPUT_CSV="time.csv"
    echo -n "Entry,sequential,sequential-dev" > "$OUTPUT_CSV"
    for t in "${THREAD_COUNTS[@]}"; do
        echo -n ",$t-avg,$t-dev" >> "$OUTPUT_CSV"
    done
    echo "" >> "$OUTPUT_CSV"

    for seqfile in "$RESULTS_DIR"/summary_seq_*.txt; do
        entry=$(basename "$seqfile" | sed 's/summary_seq_//' | sed 's/.txt//')
        parfile="$RESULTS_DIR/summary_par_${entry}.txt"
        if [[ ! -f "$parfile" ]]; then
            echo "Missing parallel result for $entry"
            continue
        fi

        seq_time=$(grep "AVG Time" "$seqfile" | awk '{print $3}')
        seq_dev=$(grep "Deviation" "$seqfile" | awk '{print $2}')
        echo -n "$entry,$seq_time,$seq_dev" >> "$OUTPUT_CSV"

        for t in "${THREAD_COUNTS[@]}"; do
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

    seq_time=$(grep "AVG Time" "$RESULTS_DIR/summary_seq_140000.txt" | awk '{print $3}')
    par_file="$RESULTS_DIR/summary_par_140000.txt"

    for t in "${THREAD_COUNTS[@]}"; do
        avg_time=$(awk -v threads=$t '
            $0 ~ "Threads: "threads {
                getline; print $3
            }
        ' "$par_file")

        if [[ -z "$avg_time" ]]; then
            echo "$t,NA,NA" >> "$OUTPUT_CSV"
        else
            efficiency=$(awk -v seq="$seq_time" -v par="$avg_time" -v p="$t" 'BEGIN { printf "%.4f", seq / (par * p) }')
            echo "$t,$avg_time,$efficiency" >> "$OUTPUT_CSV"
        fi
    done

    echo "Strong Efficiency CSV generated: $OUTPUT_CSV"
}

generate_weak_efficiency_csv() {
    OUTPUT_CSV="weak_efficiency.csv"
    echo "Threads,Size,AVG Time,Efficiency" > "$OUTPUT_CSV"

    # Tempo sequencial base: 20000 entrada
    seq_time=$(grep "AVG Time" "$RESULTS_DIR/summary_seq_20000.txt" | awk '{print $3}')

    for t in "${THREAD_COUNTS[@]}"; do
        case $t in
            2) size=20000 ;;
            4) size=40000 ;;
            8) size=80000 ;;
            12) size=120000 ;;
        esac

        par_file="$RESULTS_DIR/summary_par_${size}.txt"
        avg_time=$(awk -v threads=$t '
            $0 ~ "Threads: "threads {
                getline; print $3
            }
        ' "$par_file")

        if [[ -z "$avg_time" ]]; then
            echo "$t,$size,NA,NA" >> "$OUTPUT_CSV"
        else
            # Definição clássica de eficiência fraca:
            # Efficiency = (Base_seq_time) / (Parallel_time)
            efficiency=$(awk -v seq="$seq_time" -v par="$avg_time" 'BEGIN { printf "%.4f", seq / par }')
            echo "$t,$size,$avg_time,$efficiency" >> "$OUTPUT_CSV"
        fi
    done

    echo "Weak Efficiency CSV generated: $OUTPUT_CSV"
}

# Executa as gerações
generate_speedup_csv
generate_time_csv
generate_strong_efficiency_csv
generate_weak_efficiency_csv
