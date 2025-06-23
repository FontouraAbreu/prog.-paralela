#!/bin/bash
#SBATCH --job-name=amdahl_seq
#SBATCH --output=slurm_amdahl_%j.out
#SBATCH --error=slurm_amdahl_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=01:00:00

./seq ./testes/100000_A.in ./testes/100000_B.in > seq_output.txt