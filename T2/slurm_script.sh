#!/bin/bash

echo "Submitting 2 tasks job..."
jobid2=$(sbatch slurm_2.sh | awk '{print $4}')

echo "Submitting 4 tasks job... (after job $jobid2)"
jobid4=$(sbatch --dependency=afterok:$jobid2 slurm_4.sh | awk '{print $4}')

echo "Submitting 8 tasks job... (after job $jobid4)"
jobid8=$(sbatch --dependency=afterok:$jobid4 slurm_8.sh | awk '{print $4}')

echo "Submitting 16 tasks job... (after job $jobid8)"
sbatch --dependency=afterok:$jobid8 slurm_16.sh
