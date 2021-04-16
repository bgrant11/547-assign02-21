void*
KNN::wrapper_res(void* vp){
	Info2 *ip = (Info2 *) vp;
	
	auto sp = new std::vector<std::vector<KNN::nearest_point>>(KNN::merge_sort_res(ip->a, ip->r, ip->depth,
					ip->dimension));

	delete ip;
	return sp;
}

std::vector<std::vector<KNN::nearest_point>>
KNN::merge_sort_res(const std::vector<std::vector<nearest_point>> &a, 
									const Range &r, const unsigned int depth, 
												const unsigned long dimension) {
	//std::cout << "merge_sort" << std::endl;
	int rv;	

	auto &begin{r.begin};
	auto &end{r.end};

	if(end - begin == 1){
		return std::vector<std::vector<nearest_point>>{a.at(r.begin)};
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


	const std::vector<std::vector<nearest_point>> *lsp = nullptr;
	const std::vector<std::vector<nearest_point>> *rsp = nullptr;

	
	const unsigned int new_depth = depth + 1;
	
	if(depth < static_depth) {
				
		const auto *const lp = new Info2{a, left, new_depth, dimension};
		const auto *const rp = new Info2{a, right, new_depth, dimension};

		pthread_t l_tid, r_tid;
		//printf( "L_thread %d\n",depth);
		rv = pthread_create(&l_tid, nullptr, wrapper, (void*)lp); 
		assert(rv == 0);
		//printf( "R_thread %d\n",depth);	
		rv = pthread_create(&r_tid, nullptr, wrapper, (void*)rp); 
		assert(rv == 0);

		void *vp;	
	
		rv = pthread_join(l_tid, &vp); assert(rv == 0);
		lsp = (const std::vector<std::vector<nearest_point>> *) vp;
		

		rv = pthread_join(r_tid, &vp); assert(rv == 0);
		rsp = (const std::vector<std::vector<nearest_point>> *) vp;
		
	}else{
		lsp = new std::vector<std::vector<nearest_point>>(merge_sort_res(a, left, new_depth, 
																	dimension));
		rsp = new std::vector<std::vector<nearest_point>>(merge_sort_res(a, right, new_depth,
																	dimension));
	}	
	const std::vector<std::vector<nearest_point>> &l_sorted(*lsp);
	const std::vector<std::vector<nearest_point>> &r_sorted(*rsp);

	std::vector<std::vector<nearest_point>> sorted;
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
	
	/*	
	std::cout << typeid(l_it).name() << std::endl; // debug
	std::cout << typeid(l_sorted.begin()).name() << std::endl; // debug
	*/
	/*
	std::cout << "merge" << std::endl;
	
	std::cout << "debugPrint-------" << std::endl;
	for(unsigned long i = 0; i < l_sorted.size(); ++i){
		for(unsigned long j = 0; j < 2; ++j){
			std::cout << l_sorted[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;
	
	std::cout << "debugPrint-------" << std::endl;
	for(unsigned long i = 0; i < r_sorted.size(); ++i){
		for(unsigned long j = 0; j < 2; ++j){
			std::cout << r_sorted[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;
	*/

	
	while(!(l_it == l_sorted.end() && r_it == r_sorted.end())) {
		//std::cout << "it" << std::endl;	// debug	
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
			
			//std::cout << *l_it[dimension] << std::endl; // debug
			//std::cout << *r_it[dimension] << std::endl; // debug
			//std::cout << l_sorted.size() << std::endl; // debug
			//std::cout << r_sorted.size() << std::endl; // debug
			if((*l_it)[dimension].distance <= (*r_it)[dimension].distance){
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


////-------------------











cores: 4
							tree build time:	8323
							query time:		443

















----





1
thread_limit:		1
static_depth:		1
							cores: 1
							tree build time:	14707
							query time:		538
total thread count:	3
tmp thread count:	4294967295
max thread count:	1

2
thread_limit:		2
static_depth:		2
							cores: 2
							tree build time:	8142
							query time:		233
total thread count:	4
tmp thread count:	0
max thread count:	2

3
thread_limit:		3
static_depth:		3
							cores: 3
							tree build time:	8841
							query time:		192
total thread count:	10
tmp thread count:	0
max thread count:	3

4
thread_limit:		4
static_depth:		3
							cores: 4
							tree build time:	8091
							query time:		161
total thread count:	18
tmp thread count:	0
max thread count:	4




























//--------------






thread_limit:		14
static_depth:		3
							cores: 4
							tree build time:	82.004 s
							query time:		200.349 s
total thread count:	0
tmp thread count:	0
max thread count:	14






thread_limit:		14
static_depth:		3
							cores: 4
							tree build time:	192.189 s
							query time:		408.492 s
total thread count:	0
tmp thread count:	0
max thread count:	14






