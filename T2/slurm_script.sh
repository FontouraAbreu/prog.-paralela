#!/bin/bash
#SBATCH --job-name=LCS_mpi_Fontoura
#SBATCH --nodes=2
#SBATCH --ntasks=16
#SBATCH --output=lcs_mpi_%j.out
#SBATCH --error=lcs_mpi_%j.err
#SBATCH --time=00:10:00

mpirun -np 16 --map-by ppr:8:node ./par ./testes/9000_A.in ./testes/9000_B.in