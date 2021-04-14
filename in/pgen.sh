#!/bin/bash
#dataPts=1000

dist_start=2
dim_start=5

dist=3
dim=6

begin=$1
end=$2

q=5
kn=5

DIR=$(pwd)


DFILE=data_*.dat
QFILE=query_*.dat

for ((j=$begin;j<=$end;j++));
do
	#for ((k=$dim_start;k<=$dim;k++));	
	for ((k=6;k<=6;k++));
	do			
		#for ((i=dist_start;i<=$dist;i++));
		for ((i=2;i<=2;i++));
		do
			# ./training_data <num_pts> <num_dimensions> <dist_type>			
			./p_training_data $j $k $i
			#echo "DFILE $DFILE ..."
			#mv $DFILE ../pgen_tests/data_$j-$k-$i.dat
				
			# ./query_file <num_queries> <dimensions> <dist_type> <k>
			./p_query_file $q $k $i $kn
			#echo "QFILE $QFILE ..."
			#mv $QFILE ../pgen_tests/query_$q-$k-$i-$kn.dat	
			if [ $q -eq 7 ] 
			then
				q=1
			else
				q=$((q+1))
			fi
			if [ $kn -eq 11 ]
			then
				kn=1
			else
				kn=$((kn+1))
			fi
		done
	done
done



