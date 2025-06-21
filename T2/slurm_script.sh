#!/bin/bash
#SBATCH --job-name=lcs_batch
#SBATCH --output=slurm_batch_%j.out
#SBATCH --error=slurm_batch_%j.err
#SBATCH --nodes=2
#SBATCH --ntasks=12
#SBATCH --time=4:00:00
#SBATCH --cpu-freq=high

INPUT_SIZES=("50000" "80000" "100000")
NPROCS_LIST=(2 4 6 8 12)
REPS=20
INPUT_DIR="./"

for SIZE in "${INPUT_SIZES[@]}"; do
    FILE_A="${INPUT_DIR}/${SIZE}_A.in"
    FILE_B="${INPUT_DIR}/${SIZE}_B.in"

    SEQ_RESULTS=()

    # Inicializa arrays de tempos para cada configuração paralela
    declare -A PAR_TIMES
    for NP in "${NPROCS_LIST[@]}"; do
        PAR_TIMES[$NP]=""
    done

    for REP in $(seq 1 $REPS); do
        echo "============================================"
        echo "Run $REP - Input Size: $SIZE - Sequential"
        echo "============================================"

        SEQ_TIME=$(./seq "$FILE_A" "$FILE_B" | grep "TotalTime:" | awk '{print $2}')
        SEQ_RESULTS+=("$SEQ_TIME")

        for NP in "${NPROCS_LIST[@]}"; do
            echo "--------------------------------------------"
            echo "Run $REP - Input Size: $SIZE - MPI with $NP processes"
            echo "--------------------------------------------"

            PAR_TIME=$(mpirun -np $NP --map-by ppr:6:node ./par "$FILE_A" "$FILE_B" | grep "TotalTime:" | awk '{print $2}')
            PAR_TIMES[$NP]="${PAR_TIMES[$NP]} $PAR_TIME"
        done
    done

    # Gerar resumo de sequencial
    SUMMARY_SEQ="summary_seq_${SIZE}.txt"
    echo "Gerando resumo sequencial: $SUMMARY_SEQ"

    echo "Threads: 1" > "$SUMMARY_SEQ"
    AVG=$(echo "${SEQ_RESULTS[@]}" | awk '{for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
    STD=$(echo "${SEQ_RESULTS[@]}" | awk -v avg="$AVG" '{for(i=1;i<=NF;i++) sum+=($i-avg)^2; print sqrt(sum/NF)}')
    printf "AVG Time: %.6f seconds\n" "$AVG" >> "$SUMMARY_SEQ"
    printf "Deviation: %.6f\n\n" "$STD" >> "$SUMMARY_SEQ"

    # Gerar resumo paralelo
    SUMMARY_PAR="summary_par_${SIZE}.txt"
    echo "Gerando resumo paralelo: $SUMMARY_PAR"
    echo "" > "$SUMMARY_PAR"

    for NP in "${NPROCS_LIST[@]}"; do
        TIMES="${PAR_TIMES[$NP]}"
        AVG=$(echo $TIMES | awk '{for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
        STD=$(echo $TIMES | awk -v avg="$AVG" '{for(i=1;i<=NF;i++) sum+=($i-avg)^2; print sqrt(sum/NF)}')

        echo "Threads: $NP" >> "$SUMMARY_PAR"
        printf "AVG Time: %.6f seconds\n" "$AVG" >> "$SUMMARY_PAR"
        printf "Deviation: %.6f\n\n" "$STD" >> "$SUMMARY_PAR"
    done

    # Limpa arrays antes de começar o próximo tamanho
    unset SEQ_RESULTS
    unset PAR_TIMES
done
