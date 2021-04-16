
all: parallel_gen

gen: query data

.PHONY: query
# ./query_file <num_queries> <dimensions> <dist_type> <k>
# dist_type = unif, centered_unif, beta, exponential
query: query_file
	#./query_file 5 2 0 2
	#./query_file 5 5 2 5
	./query_file 3 6 2 4
	

.PHONY: data
# ./training_data <num_pts> <num_dimensions> <dist_type>
# dist_type = unif, centered_unif, beta, exponential
data: training_data
	#./training_datma 19 2 0
	#./training_data 10000 5 2
	./training_data 30000014 6 2


parallel_gen: parallel_gen.o
	g++ -std=c++11  -Wall -Wextra -pedantic parallel_gen.o -o parallel_gen -pthread

parallel_gen.o: parallel_gen.cpp
	g++ -std=c++11  -Wall -Wextra -pedantic -c parallel_gen.cpp

pgen: parallel_gen
	./parallel_gen

script: pgen_script.sh
	./pgen_script.sh

clean:
	rm parallel_gen *.o

cleanpgen:
	rm pgen_res/result*





new-large-gen:
	./p_training_data 10000000 7 2
	./p_query_file 100000 7 2 7

# 22171525
# q = 5
# d = 2
# dist = beta (both)
# k = 5
# t = 20 mill

# 22173642
# q = 5
# d = 2
# dist = beta (both)
# k = 5
# t = 2000

# 22174247
# q = 5
# d = 2
# dist = beta (both)
# k = 5
# t = 20
