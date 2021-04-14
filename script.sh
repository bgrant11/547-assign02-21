#!/bin/bash
#SBATCH --job-name=project2
# Number of cores
#SBATCH --cpus-per-task=25
# Number of Nodes 
#SBATCH -N 1
# Assign memory usage in Megabytes 
#SBATCH --mem=8G
#SBATCH --sockets-per-node=2
#SBATCH--cores-per-socket=6
#SBATCH --threads-per-core=2

# Run for n trials for n threads


make srun1


