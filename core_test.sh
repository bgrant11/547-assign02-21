#!/bin/bash
#SBATCH --job-name=project2
# Number of cores
#SBATCH --cpus-per-task=24
# Number of Nodes 
#SBATCH -N 1
# Assign memory usage in Megabytes 
#SBATCH --mem=8G


dataPts=30000031
dist=3
dim=6

q=1
kn=1

DIR=$(pwd)/in/pgen_tests
RES=$(pwd)/in/pgen_res

#DFILE=$DIR/data_*.dat
#QFILE=$DIR/query_*.dat

start=$1
end=$2


for((z=1; z<=24; z++));
do
	echo "./k-nn $z data_2000001-6-2.dat query_3-6-2-3.dat result_$z-2000001-6-2-3-3.dat"			
	srun ./k-nn $z $DIR/data_2000001-6-2.dat $DIR/query_3-6-2-3.dat $RES/result_$z-2000001-6-2-3-3.dat

done

rm in/pgen_res/result*
