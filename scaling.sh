#!/bin/bash


#SBATCH --job-name=brian
#SBATCH --output=scaling-out2.txt
#SBATCH -n 1
#SBATCH --cpus-per-task=24
#SBATCH --mem=8G 


exec > scaling-out.txt
cores=8
for ((i=1;i<=$cores;i++));
		do


	echo $i
	./k-nn $i in/pgen_tests/data_1000000-7-2.dat in/pgen_tests/query_100-7-2-7.dat in/new.dat
	#./k-nn $i in/pgen_tests/data_10000000-7-2.dat in/pgen_tests/query_100000-7-2-7.dat in/new.dat
	rm -f in/new.dat
done
