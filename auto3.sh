#!/bin/bash
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



for((z=1; z<=24; z++);
do
	for ((j=start;j<$end;j++));
	do
		for ((k=5;k<=$dim;k++));	
		do			
			for ((i=2;i<=$dist;i++));
			do
				#./training_data $j $k $i
				#echo "DFILE $DFILE ..."
				#mv $DFILE tests/data_$j-$k-$i.dat
				#./query_file $q $k $i $kn
				#echo "QFILE $QFILE ..."
				#mv $QFILE tests/query_$q-$k-$i-$kn.dat	
				echo "./k-nn $z $DIR/data_$j-$k-$i.dat $DIR/query_$q-$k-$i-$kn.dat $RES/result_$j-$k-$i-$q-$kn.txt"			
				./k-nn $z $DIR/data_$j-$k-$i.dat $DIR/query_$q-$k-$i-$kn.dat $RES/result_$j-$k-$i-$q-$kn.txt
				
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
done


