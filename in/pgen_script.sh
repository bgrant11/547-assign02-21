#!/bin/bash
#SBATCH --job-name=parallel_test_gen
# Number of cores
#SBATCH --cpus-per-task=24
# Number of Nodes 
#SBATCH -N 1
# Assign memory usage in Megabytes 
#SBATCH --mem=8G


# Run for n trials for n threads


srun ./parallel_gen


