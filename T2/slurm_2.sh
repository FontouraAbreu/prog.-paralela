#!/bin/bash
#SBATCH --job-name=lcs_2tasks
#SBATCH --output=slurm_2tasks_%j.out
#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --time=1:00:00

export THREAD_STEPS=(2)
bash ./executa_testes.sh
