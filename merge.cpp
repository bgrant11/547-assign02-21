#include <vector>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>


using array_t = std::vector<int>;
// using ssize_t = long long;

struct Range {
	const ssize_t begin, end;
};

struct Info {
	const std::vector<int> &a;
	const Range r;
	const int depth;
};

std::vector<int> 
merge_sort(const std::vector<int> &a, const Range &r, const int depth);

void *wrapper(void* vp){
	Info *ip = (Info *) vp;
	
	auto sp = new std::vector<int>(merge_sort(ip->a, ip->r, ip->depth));

	delete ip;
	return sp;
}

std::vector<int> 
merge_sort(const std::vector<int> &a, const Range &r, const int depth) {
	
	int rv;	

	auto &begin{r.begin};
	auto &end{r.end};

	if(end - begin == 1){
		return std::vector<int>{a.at(r.begin)};
	}
	
	const ssize_t size = end - begin;
	
	const Range left{begin, begin + size/2}, right{begin + size/2, end};
	assert(left.end - left.begin >= 1);
	assert(right.end - right.begin >= 1);

	//printf("left: %ld, %ld\n", left.begin, left.end);
	//printf("right: %ld, %ld\n", right.begin, right.end);


	/*
	auto l_sorted = merge_sort(a, left);
	auto r_sorted = merge_sort(a,right);
	*/


	const std::vector<int> *lsp = nullptr;
	const std::vector<int> *rsp = nullptr;

	
	const auto new_depth = depth + 1;
	
	if(depth < 8) {
				
		const auto *const lp = new Info{a, left, new_depth};
		const auto *const rp = new Info{a, right, new_depth};

		pthread_t l_tid, r_tid;
		std::cout << "L_thread " << depth << std::endl;
		rv = pthread_create(&l_tid, nullptr, wrapper, (void*)lp); 
		assert(rv == 0);
		std::cout << "R_thread " << depth << std::endl;		
		rv = pthread_create(&r_tid, nullptr, wrapper, (void*)rp); 
		assert(rv == 0);

		void *vp;	
	
		rv = pthread_join(l_tid, &vp); assert(rv == 0);
		lsp = (const array_t *) vp;
		

		rv = pthread_join(r_tid, &vp); assert(rv == 0);
		rsp = (const array_t *) vp;
		
	}else{
		lsp = new std::vector<int>(merge_sort(a, left, new_depth));
		rsp = new std::vector<int>(merge_sort(a, right, new_depth));
	}	
	const std::vector<int> &l_sorted(*lsp);
	const std::vector<int> &r_sorted(*rsp);

	std::vector<int> sorted;
	sorted.reserve(size);

	/*
	std::cout << "merging..." << std::endl;
	std::cout << "	left: " << std::endl;
	for(auto i : l_sorted){
		std::cout << "		" << i << std::endl;
	}
	for(auto i : r_sorted){
		std::cout << "		" << i << std::endl;
	}
	*/
	
	
	auto l_it = l_sorted.begin();
	auto r_it = r_sorted.begin();
	


	while(!(l_it == l_sorted.end() && r_it == r_sorted.end())) {
		if(r_it == r_sorted.end()) {
			assert(l_it != l_sorted.end());
			sorted.push_back(*l_it);
			++l_it;
		} else if(l_it == l_sorted.end()) {
			assert(r_it != r_sorted.end());
			sorted.push_back(*r_it);
			++r_it;
		} else {
			assert(l_it != l_sorted.end());
			assert(r_it != r_sorted.end());			
			if(*l_it <= *r_it){
				sorted.push_back(*l_it);
				++l_it;
			} else {
				sorted.push_back(*r_it);
				++r_it;
			}
		}
	}
	assert(ssize_t(sorted.size()) == size);
	assert(sorted.size() == l_sorted.size() + r_sorted.size());
	
	delete lsp;
	delete rsp;	
		
	return sorted;

}

int
main(){
	// std::vector<int> array{3, 2, 1};
	std::vector<int> array;
	for(int i = 0; i < 5000; i++){
		array.push_back(drand48()*1000);
	}
	
	auto res = merge_sort(array, Range{0, ssize_t(array.size())}, 0);
	printf("Sorted: \n");
	for(auto i : res){
		printf("%d\n", i);
	}


}
