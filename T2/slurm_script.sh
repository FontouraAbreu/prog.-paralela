#!/bin/bash
echo "Submitting 2 tasks job..."
sbatch slurm_2.sh

echo "Submitting 4 tasks job..."
sbatch slurm_4.sh

echo "Submitting 8 tasks job..."
sbatch slurm_8.sh

echo "Submitting 16 tasks job..."
sbatch slurm_16.sh
