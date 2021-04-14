#!/bin/bash
dataPts=30000100
dist=3
dim=6

q=1
kn=1

DIR=$(pwd)


DFILE=$DIR/data_*.dat
QFILE=$DIR/query_*.dat

for ((j=30000000;j<=$dataPts;j++));
do
	for ((k=5;k<=$dim;k++));	
	do			
		for ((i=2;i<=$dist;i++));
		do
			./training_data $j $k $i
			#echo "DFILE $DFILE ..."
			mv $DFILE tests/data_$j-$k-$i.dat
			./query_file $q $k $i $kn
			#echo "QFILE $QFILE ..."
			mv $QFILE tests/query_$q-$k-$i-$kn.dat	
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

cd ..
cd ..
./auto2.sh &> out2.txt



