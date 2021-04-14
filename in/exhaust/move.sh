#!/bin/bash
dataPts=30000100
dist=3
dim=6

q=1
kn=1

DIR=$(pwd)
FILES=$DIR/data_*.dat

#DFILE=$DIR/data_*.dat
#QFILE=$DIR/query_*.dat


j=29999999
k=5
i=2

count=0




if [ 1 -eq 1 ]
then
	for f in $FILES
	do
		echo "Processing $f file..."
		# take action on each file. $f store current file name
		#cat $f
	
		if [ $count -eq 0 ]
		then
			k=5
			i=2
			count=1
			j=$((j+1))
		elif [ $count -eq 1 ]
		then
			k=5
			i=3
			count=2
		elif [ $count -eq 2 ]
		then
			k=6
			i=2
			count=3
		elif [ $count -eq 3 ]
		then			
			k=6
			i=3
			count=0
		fi
		
	
		echo "data_$j-$k-$i.dat"
		mv $f tests/data_$j-$k-$i.dat
	done

		#if [ count_i -eq 0 ]
		#then
		#	if [ count_k -eq 0 ]
		#	then 
		
		
		
		#mv $f tests/data_$j-$k-$i.dat
		
fi


if [ 1 -eq 0 ];
then
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
	./auto2.sh &> out2.txt;
fi


