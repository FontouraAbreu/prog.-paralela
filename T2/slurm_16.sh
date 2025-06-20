#!/bin/bash
#SBATCH --job-name=lcs_16tasks
#SBATCH --output=slurm_16tasks_%j.out
#SBATCH --error=slurm_16tasks_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=16
#SBATCH --time=1:00:00

export THREAD_STEPS=(16)
bash ./executa_testes.sh
