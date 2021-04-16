BUILDID=$(shell date +%m/%d/%Y-%H:%M:%S)



.PHONY: all
all: k-nn

open: driver.cpp knnClass.hpp knnClass.cpp makefile
	gedit driver.cpp &
	gedit knnClass.hpp &
	gedit knnClass.cpp &
	gedit makefile &

git: clean
	git add -A .
	git commit -m "commit on $(BUILDID)"
	git push


k-nn: driver.o knnClass.o merge
	#g++ -O4 -g -Wall -Wextra -pedantic -std=c++11  driver.o -o k-nn -pthread
	#g++ -O4 -std=c++11  driver.o -o k-nn -pthread
	g++ -std=c++11 -g -v -Wall -Wextra -pedantic driver.o knnClass.o -o k-nn -pthread

driver.o: driver.cpp
	#g++ -O4 -g -Wall -Wextra -pedantic  -std=c++11 -c driver.cpp
	#g++ -O4 -std=c++11 -c driver.cpp
	g++ -g -v -Wall -Wextra -pedantic -std=c++11 -c driver.cpp


knnClass.o: knnClass.cpp
	#g++ -O4 -g -Wall -Wextra -pedantic  -std=c++11 -c driver.cpp
	#g++ -O4 -std=c++11 -c driver.cpp
	g++ -v -g -Wall -Wextra -pedantic -std=c++11 -c knnClass.cpp



new: k-nn
	./k-nn 4 in/pgen_tests/data_10010-6-2.dat in/pgen_tests/query_7-6-2-7.dat in/new.dat
	rm -f in/new.dat

new-script: new.sh
	./new.sh 10000 10010 

new-large: k-nn
	#./k-nn 1 in/pgen_tests/data_1000000-7-2.dat in/pgen_tests/query_100-7-2-7.dat in/new.dat
	./k-nn 4 in/pgen_tests/data_10000000-7-2.dat in/pgen_tests/query_100000-7-2-7.dat in/new.dat
	rm -f in/new.dat


#  ./k-nn <cores> <train_file> <query_file> <result_file>" 

# q = 5
# d = 2
# dist = beta (both)
# k = 5
# t = 20 mill
testbig: k-nn
	./k-nn 4 in/i/data_22171525.dat in/i/query_22171525.dat in/resultbig


# q = 5
# d = 2
# dist = beta (both)
# k = 5
# t = 2000
test2: k-nn
	./k-nn 4 in/i/data_22173642.dat in/i/query_22173642.dat in/result2

# q = 5
# d = 2
# dist = beta (both)
# k = 5
# t = 20
test3: k-nn
	./k-nn 4 in/i/data_22174247.dat in/i/query_22174247.dat in/result3


# q = 5
# d = 5
# dist = beta (both)
# k = 5
# t = 20

test4: k-nn
	./k-nn 4 in/i/data_25211745.dat in/i/query_25211745.dat in/result4


# q = 5
# d = 5
# dist = beta (both)
# k = 5
# t = 2 mill

test5: k-nn
	./k-nn 2 in/i/data_11094451.dat in/i/query_11094451.dat in/result5


# q = 5
# d = 5
# dist = beta (both)
# k = 5
# t = 10 mill

test6: k-nn
	./k-nn 4 in/i/data_11102600.dat in/i/query_11102600.dat in/result6


# q = 5
# d = 5
# dist = beta (both)
# k = 5
# t = 20 mill  SECOND 20 mill

test7: k-nn
	./k-nn 4 in/i/data_11103854.dat in/i/query_11103854.dat in/result7


#segfault
test8: k-nn
	./k-nn 4 in/i/data_13112102.dat in/i/query_13112102.dat in/result_30000014-6-2-3-4


#segfault
test9: k-nn
	./k-nn 4 in/exhaust/tests/data_263-1-2.dat in/exhaust/tests/query_5-1-2-10.dat in/result_263-1-2-5-10.txt

test10: k-nn
	./k-nn 4 in/exhaust/tests/data_451-1-3.dat in/exhaust/tests/query_3-1-3-2.dat in/result_451-1-3-3-2.txt



val: k-nn
	
	#valgrind --leak-check=full --show-leak-kinds=all ./k-nn 4 in/data_22174247.dat in/query_22174247.dat in/result3
	#valgrind -v --leak-check=full --track-origins=yes --show-leak-kinds=all ./k-nn 4 in/i/data_22173642.dat in/i/query_22173642.dat in/result2
	valgrind -v --leak-check=full --track-origins=yes --show-leak-kinds=all ./k-nn 4 in/pgen_tests/data_10010-6-2.dat in/pgen_tests/query_7-6-2-7.dat in/new.dat
	rm -f in/new.dat
gdb: k-nn

#	gdb --args k-nn 4 in/i/data_22171525.dat in/i/query_22171525.dat in/resultbig
	#gdb --args k-nn 4 in/exhaust/tests/data_1-3-2.dat in/exhaust/tests/query_4-3-2-11.dat in/exhaust/res/result_1-3-2-4-11.txt
	#gdb --args k-nn 4 in/i/data_13112102.dat in/i/query_13112102.dat in/result_30000014-6-2-3-4
	#gdb --args k-nn 4 in/exhaust/tests/data_263-1-2.dat in/exhaust/tests/query_5-1-2-10.dat in/result_263-1-2-5-10.txt
	#gdb --args k-nn 4 in/exhaust/tests/data_451-1-3.dat in/exhaust/tests/query_3-1-3-2.dat in/result_451-1-3-3-2.txt
	#gdb --args k-nn 2 in/i/data_11094451.dat in/i/query_11094451.dat in/result5
	
	#21 gdb --args k-nn 4 in/i/data_22174247.dat in/i/query_22174247.dat in/result3
	#gdb --args k-nn 1 in/pgen_tests/data_1000000-7-2.dat in/pgen_tests/query_100-7-2-7.dat in/new.dat

	gdb --args k-nn 4 in/pgen_tests/data_10000000-7-2.dat in/pgen_tests/query_100000-7-2-7.dat in/new.dat


inter:
	srun -N1 --pty bash -i

gdbc: k-nn
	gdb --args k-nn 3 in/pgen_tests/data_2000001-6-2.dat in/pgen_tests/query_3-6-2-3.dat in/pgen_res/result_3-2000001-6-2-3-3.dat

.PHONY: query
# ./query_file <num_queries> <dimensions> <dist_type> <k>
# dist_type = unif, centered_unif, beta, exponential
query: query_file
	./query_file 5 2 0 2
	

.PHONY: data
# ./training_data <num_pts> <num_dimensions> <dist_type>
# dist_type = unif, centered_unif, beta, exponential
data: training_data
	./training_data 19 2 0


srun1: k-nn
	srun ./k-nn 4 data_19075446.dat query_19085331.dat result1


.PHONY: clean
clean:
	rm -f *.o k-nn merge slurm-*.out in/exhaust/res/* in/pgen_res/result* in/result*

#.PHONY: cleanq
#cleanq:
#	rm query_*.dat

#.PHONY: cleand
#cleand:
#	rm data_*.dat 

.PHONY: cleanr
cleanr:
	rm result1

.PHONY: cleanin
cleanin:	
	rm in/result*





merge: merge.cpp
	g++ -Wall -Wextra -pedantic -std=c++1z -o merge merge.cpp -pthread

sort: merge
	./merge

valsort: merge
	valgrind --leak-check=full ./merge

