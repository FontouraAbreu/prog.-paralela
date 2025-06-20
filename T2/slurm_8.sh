#!/bin/bash
#SBATCH --job-name=lcs_8tasks
#SBATCH --output=slurm_8tasks_%j.out
#SBATCH --error=slurm_8tasks_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --time=1:00:00

export THREAD_STEPS=(8)
bash ./executa_testes.sh
