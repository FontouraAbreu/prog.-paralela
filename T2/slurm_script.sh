#!/bin/bash
#SBATCH --job-name=lcs_fontoura
#SBATCH --output=lcs_test_%j.out
#SBATCH --nodes=1
#SBATCH --ntasks=16
#SBATCH --time=1:00:00

bash ./executa_testes.sh
