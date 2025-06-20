#!/bin/bash
#SBATCH --job-name=lcs_4tasks
#SBATCH --output=slurm_4tasks_%j.out
#SBATCH --error=slurm_4tasks_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --time=1:00:00

export THREAD_STEPS=(4)
bash ./executa_testes.sh
