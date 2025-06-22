#!/bin/bash
#SBATCH --job-name=lcs_batch
#SBATCH --output=slurm_batch_%j.out
#SBATCH --error=slurm_batch_%j.err
#SBATCH --nodes=2
#SBATCH --ntasks=12
#SBATCH --time=4:00:00
#SBATCH --cpu-freq=high

INPUT_SIZES=("10000" "20000" "30000" "40000" "50000" "60000" "70000" "80000" "90000" "100000" "110000" "120000" "130000" "140000" "150000")
NPROCS_LIST=(2 4 8 10 12)
REPS=10
INPUT_DIR="./testes"

for SIZE in "${INPUT_SIZES[@]}"; do
    FILE_A="${INPUT_DIR}/${SIZE}_A.in"
    FILE_B="${INPUT_DIR}/${SIZE}_B.in"

    SEQ_RESULTS=()

    declare -A PAR_TOTAL_TIMES
    declare -A PAR_CALC_TIMES
    declare -A PAR_COMM_TIMES

    for NP in "${NPROCS_LIST[@]}"; do
        PAR_TOTAL_TIMES[$NP]=""
        PAR_CALC_TIMES[$NP]=""
        PAR_COMM_TIMES[$NP]=""
    done

    for REP in $(seq 1 $REPS); do
        echo "============================================"
        echo "Run $REP - Input Size: $SIZE - Sequential"
        echo "============================================"

        SEQ_TIME=$(./seq "$FILE_A" "$FILE_B" | grep "Time:" | awk '{print $2}')
        echo "SeqTime: $SEQ_TIME"
        SEQ_RESULTS+=("$SEQ_TIME")

        for NP in "${NPROCS_LIST[@]}"; do
            echo "--------------------------------------------"
            echo "Run $REP - Input Size: $SIZE - MPI with $NP processes"
            echo "--------------------------------------------"

            OUTPUT=$(mpirun -np $NP ./par "$FILE_A" "$FILE_B")

            TOTAL_TIME=$(echo "$OUTPUT" | grep "TotalTime:" | awk '{print $2}')
            CALC_TIME=$(echo "$OUTPUT" | grep "CalcTime:" | awk '{print $2}')
            COMM_TIME=$(echo "$OUTPUT" | grep "CommTime:" | awk '{print $2}')

            echo "Total: $TOTAL_TIME | Calc: $CALC_TIME | Comm: $COMM_TIME"

            PAR_TOTAL_TIMES[$NP]="${PAR_TOTAL_TIMES[$NP]} $TOTAL_TIME"
            PAR_CALC_TIMES[$NP]="${PAR_CALC_TIMES[$NP]} $CALC_TIME"
            PAR_COMM_TIMES[$NP]="${PAR_COMM_TIMES[$NP]} $COMM_TIME"
        done
    done

    # ==== Salvar resumo sequencial ====
    SUMMARY_SEQ="summary_seq_${SIZE}.txt"
    echo "Gerando resumo sequencial: $SUMMARY_SEQ"
    echo "Threads: 1" > "$SUMMARY_SEQ"

    AVG=$(echo "${SEQ_RESULTS[@]}" | awk '{for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
    STD=$(echo "${SEQ_RESULTS[@]}" | awk -v avg="$AVG" '{for(i=1;i<=NF;i++) sum+=($i-avg)^2; print sqrt(sum/NF)}')
    printf "AVG Time: %.6f seconds\n" "$AVG" >> "$SUMMARY_SEQ"
    printf "Deviation: %.6f\n\n" "$STD" >> "$SUMMARY_SEQ"

    # ==== Salvar resumo paralelo ====
    SUMMARY_PAR="summary_par_${SIZE}.txt"
    echo "Gerando resumo paralelo: $SUMMARY_PAR"
    echo "" > "$SUMMARY_PAR"

    for NP in "${NPROCS_LIST[@]}"; do
        TOTALS="${PAR_TOTAL_TIMES[$NP]}"
        CALCS="${PAR_CALC_TIMES[$NP]}"
        COMMS="${PAR_COMM_TIMES[$NP]}"

        # Média e desvio padrão TotalTime
        AVG_TOTAL=$(echo $TOTALS | awk '{for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
        STD_TOTAL=$(echo $TOTALS | awk -v avg="$AVG_TOTAL" '{for(i=1;i<=NF;i++) sum+=($i-avg)^2; print sqrt(sum/NF)}')

        # Média e desvio padrão CalcTime
        AVG_CALC=$(echo $CALCS | awk '{for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
        STD_CALC=$(echo $CALCS | awk -v avg="$AVG_CALC" '{for(i=1;i<=NF;i++) sum+=($i-avg)^2; print sqrt(sum/NF)}')

        # Média e desvio padrão CommTime
        AVG_COMM=$(echo $COMMS | awk '{for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
        STD_COMM=$(echo $COMMS | awk -v avg="$AVG_COMM" '{for(i=1;i<=NF;i++) sum+=($i-avg)^2; print sqrt(sum/NF)}')

        echo "Threads: $NP" >> "$SUMMARY_PAR"
        printf "AVG Time: %.6f seconds\n" "$AVG_TOTAL" >> "$SUMMARY_PAR"
        printf "AVG Calc: %.6f seconds\n" "$AVG_CALC" >> "$SUMMARY_PAR"
        printf "AVG COMM: %.6f seconds\n" "$AVG_COMM" >> "$SUMMARY_PAR"
        printf "Deviation: %.6f\n\n" "$STD_TOTAL" >> "$SUMMARY_PAR"
    done

    # Limpa arrays antes do próximo tamanho
    unset SEQ_RESULTS
    unset PAR_TOTAL_TIMES
    unset PAR_CALC_TIMES
    unset PAR_COMM_TIMES
done
