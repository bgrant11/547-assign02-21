#!/bin/bash
dataPts=1000
dist=3
dim=6

q=1
kn=1

DIR=$(pwd)/in/exhaust/tests
RES=$(pwd)/in/exhaust/res

DFILE=$DIR/data_*.dat
QFILE=$DIR/query_*.dat

for ((j=1;j<=$dataPts;j++));
do
	for ((k=1;k<=$dim;k++));	
	do			
		for ((i=0;i<=$dist;i++));
		do
			#./training_data $j $k $i
			#echo "DFILE $DFILE ..."
			#mv $DFILE tests/data_$j-$k-$i.dat
			#./query_file $q $k $i $kn
			#echo "QFILE $QFILE ..."
			#mv $QFILE tests/query_$q-$k-$i-$kn.dat	
			#echo "./k-nn 4 $DIR/data_$j-$k-$i.dat $DIR/query_$q-$k-$i-$kn.dat $RES/result_$j-$k-$i-$q-$kn.txt"			
			./k-nn 4 $DIR/data_$j-$k-$i.dat $DIR/query_$q-$k-$i-$kn.dat $RES/result_$j-$k-$i-$q-$kn.txt
			
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


