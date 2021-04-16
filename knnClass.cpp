#include "knnClass.hpp"
//std::atomic<unsigned long> thread_count{0};
unsigned int KNN::static_depth;
//pthread_mutex_t KNN::lock;
pthread_spinlock_t KNN::spinlock;

pthread_spinlock_t KNN::thread_lock;

unsigned int KNN::global_thread_id = 0;
unsigned int KNN::mod_logical_hw_cores = std::thread::hardware_concurrency()/2;
unsigned int KNN::static_cores;
unsigned int KNN::static_total_threads = 0;
unsigned int KNN::tmp_thread_count = 0;
unsigned int KNN::max_threads = 0;
unsigned int KNN::thread_limit;

//std::atomic<unsigned long> KNN::thread_count = 0;
//std::atomic_init(KNN::thread_count, 0);
KNN::~KNN(){
	this->delete_nodes(this->root);
	delete[] this->results;
	free(result_f_name);
	//pthread_mutex_destroy(&KNN::lock);
	pthread_spin_destroy(&KNN::spinlock);
}

void KNN::delete_nodes(Node* node){
	if(node){
		this->delete_nodes(node->l_child);
		this->delete_nodes(node->r_child);
		delete node;
	}
}
/*
void KNN::make_tree(){
	unsigned long idx = 0;
	for(unsigned int i = 0; i < this->num_training_pts; ++i){
		for(unsigned int j = 0; j < this->num_dimensions; ++j){
			std::cout << this->train_pts[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;	
	
	qsort_r(this->train_pts, (size_t)this->num_training_pts, 
						(sizeof(double**)), this->compare_by_dimension, &idx);
	for(unsigned int i = 0; i < this->num_training_pts; ++i){
		for(unsigned int j = 0; j < this->num_dimensions; ++j){
			std::cout << this->train_pts[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;
}

*/

void KNN::make_tree(){
	this->set_depth_init_mutex();
	//std::cout << "make tree" << std::endl;	// debug
	//std::cout << "KNN::mod_logical_hw_cores: " << KNN::mod_logical_hw_cores << std::endl;
	unsigned long dim = 0;
	unsigned long depth = 0;	
	std::vector<float*> t_pts;
	t_pts.reserve(this->num_training_pts);
	this->cpy_to_vect(t_pts);
	//this->_print(t_pts);	// debug
	if(t_pts.size() == 0){
		std::cout << "Can't make tree with zero points" << std::endl;
		exit(0);
	}
	if(t_pts.size() == 1){
		this->root = new Node(0, 0.0, 1);
		this->root->point = t_pts[0];
		return;
	}

	//std::cout << "total threads before: " << KNN::static_total_threads << std::endl;
	
	//auto merge_begin = std::chrono::steady_clock::now();
	
	std::vector<float*> a = 
		this->merge_sort(t_pts, Range{0, ssize_t(t_pts.size()/4)}, depth, dim, 
																		false);
			
	//auto merge_end = std::chrono::steady_clock::now();
	//auto merge_interval = 
	//			std::chrono::duration_cast<std::chrono::milliseconds>
	//			(merge_end-merge_begin);
	//21 std::cout << "mergesort time:\t\t" << merge_interval.count() << std::endl;
	//21 std::cout << "total msort threads:\t" << KNN::static_total_threads << std::endl;	
	//std::cout << "depth: " << depth << std::endl;	
	
	//exit(0);
	
	//std::cout << t_pts.capacity() << std::endl; // debug
	//this->_print(a); // debug
	
	std::vector<float*> left;
	std::vector<float*> right;	

	//auto split_begin = std::chrono::steady_clock::now();
	float median = this->get_median_and_split(a, dim);
	for(unsigned long i = 0; i < this->num_training_pts; i++){
		if(t_pts[i][0] < median){
			left.push_back(t_pts[i]);
		} else{
			right.push_back(t_pts[i]);
		}
	}	
	//auto split_end = std::chrono::steady_clock::now();
	//auto split_interval = 
	//			std::chrono::duration_cast<std::chrono::milliseconds>
	//			(split_end-split_begin);
	//std::cout << "split time: " << split_interval.count() << std::endl;
	
	//std::cout << "median: " << median << std::endl; // debug
	
	//this->_print(left); // debug
	//this->_print(right); // deubg 
	this->root = new Node(dim, median, this->num_training_pts);
	++depth;
	unsigned long new_dim = this->cycle_dimensions(dim);	
	
	const node_info* const l_info = 
					new node_info{left, depth, new_dim, this->root->l_child,
														this->root, this, true};
	const node_info* const r_info = 
					new node_info{right, depth, new_dim, this->root->r_child,
														this->root, this, true};
	
	
	pthread_t l_tid, r_tid;
	int rv;	
	//std::cout << "L_thread " << depth << std::endl; // debug
	bool made_left = KNN::make_thread(); 
	bool made_right = KNN::make_thread();
	if(made_left){ // dont need if....just for consistency
		rv = pthread_create(&l_tid, nullptr, populate_nodes, (void*)l_info); 
		assert(rv == 0);
	}	
	//sleep(1);	//debug
	//std::cout << "R_thread " << depth << std::endl;	// debug
	if(made_right){	
		rv = pthread_create(&r_tid, nullptr, populate_nodes, (void*)r_info); 
		assert(rv == 0);
	}
	
	void *vp;	
	if(made_left){
		rv = pthread_join(l_tid, &vp); assert(rv == 0);
	} else{
		populate_nodes((void*)l_info);
	}
	if(made_right){		
		rv = pthread_join(r_tid, &vp); assert(rv == 0);
	} else{
		populate_nodes((void*)r_info);
	}
		
	
	
	
}

void* KNN::populate_nodes(void* vp){
	//std::cout << "populate_nodes" << std::endl; // debug	
	KNN::node_info* info = (KNN::node_info*)vp;	

	
	bool affinity = false;
	//bool affinity = false;
	//int tid;
	//int rc;	
	//cpu_set_t cpuset;	

	/*
	bool affinity = false;


	if(affinity && info->is_thread){

		KNN::affinity();
	}	
	*/

	if(info->a.size() != 0){
		
		if(info->a.size() == 1){
			info->n = new Node(info->dimension, 0.0, 1);
			info->n->point = (info->a[0]);
			info->n->parent = info->parent;			
			delete info;	
			return NULL;

		}
		
		

		ssize_t local_end;
		if(info->a.size() < 100){
			local_end = info->a.size();
		} else{
			local_end = info->a.size()/4;
		}		
		//info->obj->_print(info->a); // debug
		std::vector<float*> a_sorted = info->obj->merge_sort(info->a, 
											Range{0, ssize_t(local_end)},
											info->depth,
											info->dimension, false);
		//info->obj->_print(a_sorted); // debug
		std::vector<float*> left;
		std::vector<float*> right;	
	
		float median = info->obj->get_median_and_split(a_sorted, 
														info->dimension);
		
		for(unsigned long i = 0; i < info->a.size(); i++){
		if(info->a[i][info->dimension] < median){
			left.push_back(info->a[i]);
		} else{
			right.push_back(info->a[i]);
		}
	}	
		
		//std::cout << median << std::endl; // debug
		//info->obj->_print(left); // debug
		//info->obj->_print(right); // deubg 
		info->n = new Node(info->dimension, median, a_sorted.size());
		info->n->parent = info->parent;			
		unsigned long new_depth = info->depth + 1;
		unsigned long new_dim = info->obj->cycle_dimensions(info->dimension);	

		if(info->depth < static_depth){	

			pthread_t l_tid, r_tid;
			int rv;	
			
			bool l = false;
			bool r = false;
			if(KNN::make_thread()){
				//std::cout << "L_thread " << new_depth << std::endl; // debug
				const node_info* const l_info = 
					new node_info{left, new_depth, new_dim, info->n->l_child,
													info->n, info->obj, true};				
				l = true;				
				rv = pthread_create(&l_tid, nullptr, populate_nodes, (void*)l_info); 
				//assert(rv == 0);
			} else{
				const node_info* const l_info = 
					new node_info{left, new_depth, new_dim, info->n->l_child,
													info->n, info->obj, false};				
				populate_nodes((void*)l_info);
			}
			
			if(KNN::make_thread()){
				//std::cout << "R_thread " << new_depth << std::endl;	// debug
				const node_info* const r_info = 
					new node_info{right, new_depth, new_dim, info->n->r_child,
													info->n, info->obj, true};				

				r = true;
				rv = pthread_create(&r_tid, nullptr, populate_nodes, (void*)r_info); 
				//assert(rv == 0);
			} else{
				const node_info* const r_info = 
					new node_info{right, new_depth, new_dim, info->n->r_child,
													info->n, info->obj, false};				
				populate_nodes((void*)r_info);
			}
			void *vp;	
			if(l){
				rv = pthread_join(l_tid, &vp); assert(rv == 0);
			}
			if(r){			
				rv = pthread_join(r_tid, &vp); assert(rv == 0);
			}
		}else{
			const node_info* const l_info = 
					new node_info{left, new_depth, new_dim, info->n->l_child,
													info->n, info->obj, false};
			const node_info* const r_info = 
					new node_info{right, new_depth, new_dim, info->n->r_child,
													info->n, info->obj, false};			
					
			populate_nodes((void*)l_info);
			populate_nodes((void*)r_info);
		}
	}
	if(info->is_thread) KNN::decrement_tmp_threads();
	delete info;	
	return NULL;
}

void KNN::do_queries(){
	int rv;	
	
	//this->_print_tree(this->root, 0); // debug	
	//exit(0);
	
	unsigned long * num_found = new unsigned long[this->num_queries]();


	//std::cout << "do_queries" << std::endl;

	//std::cout << "k= " << this->k << std::endl;
	std::vector<std::vector<float>> closest_dim_pt
			(this->num_queries, std::vector<float>(this->num_dimensions, 0.0));

	float* closest_dim_pt_dist = new float[this->num_queries];
	
	query_args** q_args_arr = new query_args*[this->num_queries];
	std::vector<query_args*> work;
	for(unsigned long i = 0; i < this->num_queries; ++i){	
		closest_dim_pt_dist[i] = 
						this->distance(this->query_pts[i], closest_dim_pt[i]);
		q_args_arr[i] = new query_args{this->query_pts[i], num_found[i], 
								this->root, this, nullptr, nullptr, 
								//new std::priority_queue<nearest_point, 
								//std::vector<nearest_point>, priority_compare>};
								new result_queue, 
								//std::vector<float>(this->num_dimensions, 0.0)};
								closest_dim_pt[i],								
								closest_dim_pt_dist[i]};
		work.push_back(q_args_arr[i]);

	}
	unsigned int job_cores = this->cores * 2; // = 1;
	pthread_t* workers = new pthread_t[job_cores];

	work_info** w_info = new work_info*[job_cores];
	for(unsigned int i = 0; i < job_cores; i++){
		w_info[i] = new work_info{i, work};	
		rv = pthread_create(&(workers[i]), nullptr, adhoc_worker, (void*)(w_info[i]));
		//if(rv != 0){
		//	std::cout << "problem creating worker thread" << std::endl;
		//	exit(1);
		//}
	}
	for(unsigned long i = 0; i < job_cores; i++){
		rv = pthread_join(workers[i], nullptr);
		//if(rv != 0){
		//	std::cout << "problem joining workers" << std::endl;
		//	exit(1);
		//}
	}
	//21 std::cout << "after join" << std::endl;

	this->results = new std::vector<nearest_point>[this->num_queries];
	for(unsigned long i = 0; i < this->num_queries; ++i){
		for(unsigned long j = 0; j < this->k; ++j){
			this->results[i].push_back(q_args_arr[i]->queue->top());
			q_args_arr[i]->queue->pop();
		}
	}


	//std::cout << "end queries" << std::endl;	
	

	this->results_to_file();
	delete[] num_found;
	delete[] closest_dim_pt_dist;
	for(unsigned int i = 0; i < this->num_queries; i++){
		delete q_args_arr[i]->queue;	
		delete q_args_arr[i];
	}
	for(unsigned int i = 0; i < job_cores; i++){
		delete w_info[i];
	}
	delete[] w_info;
	delete[] q_args_arr;
	delete[] workers;
	

	

}

void* KNN::adhoc_worker(void* vp){
	//std::cout << "in adhoc" << std::endl;
	int rv;	
	unsigned long idx;	
	work_info* w_info = (work_info*)vp;
	///*	
	cpu_set_t cpuset;	
	CPU_ZERO(&cpuset);
	CPU_SET(w_info->work_core % KNN::static_cores, &cpuset);
	int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    //if (rc != 0){
    //	std::cerr << "Error setting affinity_here" << '\n';
   	//}
	//*/
	KNN::query_args * job;
	while(true){
		rv = pthread_spin_lock(&KNN::thread_lock);
		//if(rv != 0){
		//	std::cout << "problem locking in worker" << std::endl;
		//	exit(1);
		//}
		idx = w_info->work.size();
		//std::cout << "size: " << idx << std::endl;
		if(idx == 0) {
			rv = pthread_spin_unlock(&KNN::thread_lock);
			//if(rv != 0){
			//	std::cout << "problem unlocking in worker" << std::endl;
			//	exit(1);
			//}
			return nullptr;
		}
		job = w_info->work[idx-1];
		w_info->work.pop_back();
		rv = pthread_spin_unlock(&KNN::thread_lock);
		//if(rv != 0){
		//	std::cout << "problem unlocking in worker" << std::endl;
		//	exit(1);
		//}
		//std::cout << "job :" << job << std::endl;
		KNN::query_nodes((void*)(job));
		
	}
}



void* KNN::query_nodes(void* vp){
	//bool debug = false;	
	query_args* q_args = (query_args*) vp;	
	//if(debug){	
	//	std::cout << "("<< q_args->query_point[0] << ", ";
	//	std::cout << q_args->query_point[1] << "): ";	
	//}

	float distance;	
	Node* first;
	Node* second;
	query_args* first_args;
	query_args* second_args;
	bool first_alloc = false;
	bool second_alloc = false;	
	if(q_args->node->num_pts == 1){ // if at leaf
		//if(debug){		
		//	std::cout << "leaf, (";
		//	std::cout << q_args->node->point[0] << ", ";
		//	std::cout << q_args->node->point[1] << "), ";
		//}		
		distance = q_args->obj->distance(q_args->query_point, 
														q_args->node->point);		
		if(q_args->num_found < q_args->obj->k){ //grab pt if < k found
			//if(debug){			
			//	std::cout << "<k, ADD_PT, ";
			//}			
			q_args->queue->push(nearest_point{q_args->node->point, distance,
														nullptr, nullptr});
			
			q_args->num_found++;
		} else{
			//if(debug){
			//	std::cout << "has_k, distance = ";
			//	std::cout << distance << ", furthest = ";
			//	std::cout << q_args->queue->top().distance << ", ";
			//}

			if(distance < q_args->queue->top().distance){ // grab if closer than
				q_args->queue->pop();					// a current pt
				q_args->queue->push(nearest_point{q_args->node->point, distance,
														nullptr, nullptr});
				//if(debug){				
				//	std::cout << "ADD_PT" << std::endl;
				//}
			} //else{
			//	if(debug){				
			//		std::cout << "no_add_pt" << std::endl;
			//	}
			//}	
		}
	} else{ // if not at leaf
		
		float tmp = q_args->closest_dim_pt[q_args->node->dimension];
		q_args->closest_dim_pt[q_args->node->dimension] = q_args->node->median;
		float test_dist = 
				q_args->obj->distance(q_args->query_point, q_args->closest_dim_pt);
		if(test_dist < q_args->closest_dim_pt_dist){
			q_args->closest_dim_pt_dist = test_dist;
		} else {
			q_args->closest_dim_pt[q_args->node->dimension] = tmp;
		}

		//if(debug){
		//	std::cout << "not_leaf, (" << q_args->node->dimension << ", ";	
		//	std::cout << q_args->node->median << "), ";	
		//}

		if(q_args->query_point[q_args->node->dimension] < q_args->node->median){
			//if(debug){			
			//	std::cout << "left_first";	
			//}		
			
			first = q_args->node->l_child;  // determine which child to go to
			second = q_args->node->r_child;	// first
		} else{
			//if(debug){			
			//	std::cout << "right_first";	
			//}		
			first = q_args->node->r_child;
			second = q_args->node->l_child;
			
		}
		//assert(first != nullptr);
		//assert(second != nullptr);
		
		first_args = new query_args{q_args->query_point, 
								q_args->num_found, first, q_args->obj, nullptr, 
								nullptr, q_args->queue, q_args->closest_dim_pt,
								q_args->closest_dim_pt_dist};
		first_alloc = true;
		//if(debug){		
		//	std::cout << std::endl;
		//}
		
		if(first != nullptr) KNN::query_nodes((void*)first_args);
		//if(debug){		
		//	std::cout << "("<< q_args->query_point[0] << ", ";
		//	std::cout << q_args->query_point[1] << "): ";
		//	std::cout << "BACK: (" << q_args->node->dimension << ", ";	
		//	std::cout << q_args->node->median << "), ";
		//	std::cout << "num_found = " << q_args->num_found << ", ";
		//}
		
		q_args->closest_dim_pt[q_args->node->dimension] = q_args->node->median;
		q_args->closest_dim_pt_dist = 
			q_args->obj->distance(q_args->query_point, q_args->closest_dim_pt);
		if(q_args->num_found < q_args->obj->k){ // if less than k pts go to 
			//if(debug){			
			//	std::cout << "<k, descend, " << std::endl;			
			//}			
			second_args = new query_args{q_args->query_point, // other child
								q_args->num_found, second, q_args->obj, nullptr, 
								nullptr, q_args->queue, q_args->closest_dim_pt,
								q_args->closest_dim_pt_dist};
			second_alloc = true;
			if(second != nullptr) KNN::query_nodes((void*)second_args);
		} else { // if k pts but dist from query to partition < current:go other
			//if(debug){			
			//	std::cout << "has_k, dist_to_part = ";	
			//}		
		
			float dist_to_partition = q_args->query_point[q_args->node->dimension] -
						 q_args->node->median;
			dist_to_partition *=dist_to_partition;
			//std::cout << dist_to_partition << ", furthest = ";
			//if(debug){			
			//	std::cout << q_args->queue->top().distance << ", ";
			//}

			if(dist_to_partition < q_args->queue->top().distance){		
				//if(debug){
				//	std::cout << "descend" << std::endl;	
				//}
			
				second_args = new query_args{q_args->query_point, 
								q_args->num_found, second, q_args->obj, nullptr, 
								nullptr, q_args->queue, q_args->closest_dim_pt,
								q_args->closest_dim_pt_dist};
				second_alloc = true;
				if(second != nullptr) KNN::query_nodes((void*)second_args);
			}// else {
			//	if(debug){				
			//		std::cout << "go_up" << std::endl;
			//	}
			//}
			//delete[] pt; part of old test
		}
	}
	//if(debug){
	//	std::cout << "("<< q_args->query_point[0] << ", ";
	//	std::cout << q_args->query_point[1] << "), ";
	//	std::cout << "(" << q_args->node->dimension << ", ";	
	//	std::cout << q_args->node->median << "), ";
	//	std::cout << "go_up" << std::endl;
	//}
	if(first_alloc) delete first_args;
	if(second_alloc) delete second_args;	
	
	result_queue* ret = q_args->queue;

	return ret;
	
}

inline float KNN::distance(const float* query, const float* point){
	float distance = 0.0f;	
	for(unsigned long i = 0; i < this->num_dimensions; ++i){
		distance += this->square(query[i] - point[i]);
	}
	return distance;
}

inline float KNN::distance(const float* query, const std::vector<float> point){
	float distance = 0.0f;	
	for(unsigned long i = 0; i < this->num_dimensions; ++i){
		distance += this->square(query[i] - point[i]);
	}
	return distance;
}

inline float KNN::square(float a){
	return a*a;
}

inline void KNN::cpy_to_vect(std::vector<float*> &v){
	for(unsigned long i = 0; i < this->num_training_pts; ++i){
		v.push_back(this->train_pts[i]);
	}
}

void*
KNN::wrapper(void* vp){
	Info *ip = (Info *) vp;
	
	auto sp = new std::vector<float*>(KNN::merge_sort(ip->a, ip->r, ip->depth,
					ip->dimension, true));

	delete ip;
	return sp;
}

std::vector<float*> 
KNN::merge_sort(const std::vector<float*> &a, const Range &r, 
			const unsigned int depth, const unsigned long dimension, 
															bool is_thread) {
	//std::cout << "merge_sort" << std::endl;
	
	/*
	bool affinity = false;
	if(affinity && is_thread){

		KNN::affinity();
	}
	*/

	int rv;	

	auto &begin{r.begin};
	auto &end{r.end};

	if(end - begin == 1){
		std::vector<float*> z{a.at(r.begin)};		
		//return std::vector<float*>{a.at(r.begin)};
		return z;
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


	const std::vector<float*> *lsp = nullptr;
	const std::vector<float*> *rsp = nullptr;

	
	const unsigned int new_depth = depth + 1;
	
	

	if(depth < static_depth) {
				
		//const auto *const lp = new Info{a, left, new_depth, dimension};
		//const auto *const rp = new Info{a, right, new_depth, dimension};

		pthread_t l_tid, r_tid;

		
		bool l = false;
		bool r = false;
		
		if(KNN::make_thread()){
			//printf( "L_thread %d\n",depth);
			l=true;			
			const auto *const lp = new Info{a, left, new_depth, dimension};			
			rv = pthread_create(&l_tid, nullptr, wrapper, (void*)lp);
			assert(rv == 0);
		} else{
			//lsp = (const std::vector<float*> *) wrapper((void*)lp);
			lsp = new std::vector<float*>(merge_sort(a, left, new_depth, 
															dimension, false));
		}
		
		if(KNN::make_thread()){
			//printf( "R_thread %d\n",depth);	
			r=true;		
			const auto *const rp = new Info{a, right, new_depth, dimension};			
			rv = pthread_create(&r_tid, nullptr, wrapper, (void*)rp); 
			assert(rv == 0);
		} else{
			//rsp = (const std::vector<float*> *) wrapper((void*)rp);
			rsp = new std::vector<float*>(merge_sort(a, right, new_depth,
															dimension, false));
		}


		void *vp;	
		if(l){
			rv = pthread_join(l_tid, &vp); assert(rv == 0);
			lsp = (const std::vector<float*> *) vp;
		}
		if(r){
			rv = pthread_join(r_tid, &vp); assert(rv == 0);
			rsp = (const std::vector<float*> *) vp;
		}
	
	}else{
		lsp = new std::vector<float*>(merge_sort(a, left, new_depth, 
															dimension, false));
		rsp = new std::vector<float*>(merge_sort(a, right, new_depth,
															dimension, false));
	}	
	const std::vector<float*> &l_sorted(*lsp);
	const std::vector<float*> &r_sorted(*rsp);

	std::vector<float*> sorted;
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
			if((*l_it)[dimension] <= (*r_it)[dimension]){
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

	
	if(is_thread) KNN::decrement_tmp_threads();
	return sorted;

}

inline float KNN::get_median_and_split(const std::vector<float*> &a, 
					const unsigned long &dimension){
	float median;	
	unsigned long idx;	
	//std::cout << "median" << std::endl; // debug	
	//std::cout << a.size() << std::endl; // debug	
	if(a.size()%2 == 1){
		//std::cout << "in odd" << std::endl; // debug
		idx = (a.size()/2); // dont add 1 because array idx strt @ 0 		
		median = a[idx][dimension];
		//left = new std::vector<float*>(a.begin(), a.begin() + idx);
		//right = new std::vector<float*>(a.begin() + idx, a.end()); // right
	}													// should have median
	else{
		//std::cout << "in even" << std::endl; // debug
		idx = a.size()/2;
							// TODO make sure this is doing what it should
		median = (a[idx][dimension] + a[idx-1][dimension]) / 2.0f; // -1 for idx
		
	}
	
	/*	
	unsigned long right_begin_idx = 0;	
	for(unsigned long i = 0; i < a.size(); ++i){
		if(a[i][dimension] >= median) break;
		++right_begin_idx;
	}
	if(right_begin_idx == 0) {
		right.assign(a.begin(), a.end());
	} else if(right_begin_idx == a.size()){
		left.assign(a.begin(), a.end());
	} else {
		left.assign(a.begin(), a.begin() + right_begin_idx);
		right.assign(a.begin() + right_begin_idx, a.end());
	}
	*/
	
	//this->split(a, left, right, dimension, median);
	return median;
}

//not used
void KNN::split(const std::vector<float*> &a,
				std::vector<float*> &left,
				std::vector<float*> &right, 
				const unsigned long &dimension,
				const float median){
	unsigned long right_begin_idx = 0;	
	for(unsigned long i = 0; i < a.size(); ++i){
		if(a[i][dimension] >= median) break;
		++right_begin_idx;
	}
	if(right_begin_idx == 0) {
		right.assign(a.begin(), a.end());
	} else if(right_begin_idx == a.size()){
		left.assign(a.begin(), a.end());
	} else {
		left.assign(a.begin(), a.begin() + right_begin_idx);
		right.assign(a.begin() + right_begin_idx, a.end());
	}
}


inline unsigned long KNN::cycle_dimensions(unsigned long dim){
	if(dim < (this->num_dimensions-1)) return dim+1;
	return 0;
}

void KNN::results_to_file(){
	//std::cout << "reslts to file" << std::endl;	
	int fd = open(this->result_f_name, O_RDWR | O_CREAT | O_TRUNC, 
											(mode_t)0600); assert(fd != -1);
	size_t num_floats = this->num_queries * this->k * this->num_dimensions;
	size_t file_size = 56 + (4 * num_floats);
	//::cout << "filesize " << file_size << std::endl;
	if(lseek(fd, file_size-1, SEEK_SET) == -1){
		close(fd);
		std::cout << "Error on lseek of result file" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (write(fd, "", 1) == -1){
        close(fd);
        std::cout << "Error on setting result file size" << std::endl;
        exit(EXIT_FAILURE);
	}
	
	
	void *map_vp = mmap(nullptr, file_size, PROT_WRITE | PROT_READ, 
							MAP_SHARED	/*MAP_PRIVATE|MAP_POPULATE */, fd, 0);
	if (map_vp == MAP_FAILED) {
		close(fd);     	
		int en = errno;
    	fprintf(stderr, "mmap() failed: %s\n", strerror(en));   	
		exit(3);
    }
	int rv;
	rv=madvise(map_vp, file_size, MADV_SEQUENTIAL|MADV_WILLNEED); assert(rv == 0);
	char f_type[] = "RESULT";
	char * char_ptr	= (char*) map_vp;
	for(int i = 0; i < 8; ++i){
		if(i < 6) char_ptr[i] = f_type[i];
		else char_ptr[i] = '\0';
	}
	unsigned long idx = 1;	
	unsigned long* ul_ptr = (unsigned long*) map_vp;
	ul_ptr[idx] = this->train_id; idx++;
	ul_ptr[idx] = this->query_id; idx++;
	this->res_id = this->urandom();
	//std::cout << "gen res id " << this->res_id << std::endl;
	ul_ptr[idx] = this->res_id; idx++;
	ul_ptr[idx] = this->num_queries; idx++;
	ul_ptr[idx] = this->num_dimensions; idx++;
	ul_ptr[idx] = this->k; idx++;
	float* fl_ptr = (float*) map_vp; idx*=2; // 32bitfloat-> 2*idx of 64bitlong
	for(unsigned long i = 0; i < this->num_queries; ++i){
		for(long j = (this->k - 1); j >= 0; --j){
			for(unsigned long k = 0; k < this->num_dimensions; ++k){
				fl_ptr[idx] = this->results[i][j].point[k]; idx++;
			}
		}
	}
	if(msync(map_vp, file_size, MS_SYNC) == -1){
		std::cout << "Error syncing mmaped file to disk" << std::endl;
	}
	if(munmap(map_vp, file_size) == -1){
		std::cout << "Error on munmap" << std::endl;
		close(fd);
		exit(EXIT_FAILURE);
	}

	close(fd);
	
}

inline unsigned long KNN::urandom(){
	unsigned long random = 0;
	//size_t size = sizeof(random);
	std::ifstream urandom("/dev/urandom");
	if(!urandom){
		std::cout << "/dev/urandom Failed" << std::endl;
		exit(1);
	}
	urandom.read(reinterpret_cast<char*>(&random), sizeof(random));
	urandom.close();
	return random % 70000000; 		
}

/*
int KNN::get_thread_id(){
	unsigned int local_thread_id;	
	//int rv = pthread_mutex_lock(&KNN::lock);
	int rv = pthread_spin_lock(&KNN::spinlock);
	if(rv != 0){
		std::cout << "problem locking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	//if(KNN::tmp_thread_count

	//static_total_threads++;
	//KNN::tmp_thread_count++;
	//if(KNN::tmp_thread_count > KNN::max_threads){
	//	KNN::max_threads = KNN::tmp_thread_count;
	//}
	local_thread_id = KNN::global_thread_id;
	if(KNN::global_thread_id == (KNN::static_cores-1)){
		KNN::global_thread_id = KNN::mod_logical_hw_cores;
	}
	else if(KNN::global_thread_id==(KNN::static_cores-1)+KNN::mod_logical_hw_cores){
		global_thread_id = 0;
	}
	else {
		++global_thread_id;
	}
	std::cout << "thread id " << local_thread_id << std::endl;	
		
	//rv = pthread_mutex_unlock(&KNN::lock);
	rv = pthread_spin_unlock(&KNN::spinlock);
	if(rv != 0){
		std::cout << "problem unlocking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
		
	return local_thread_id;
	
}
*/

int KNN::get_thread_id(){
	unsigned int local_thread_id;	
	unsigned int tmp;
	//int rv = pthread_mutex_lock(&KNN::lock);
	int rv = pthread_spin_lock(&KNN::spinlock);
	if(rv != 0){
		std::cout << "problem locking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	tmp = KNN::static_total_threads % KNN::static_cores*2;
	if(tmp%2==0){
		local_thread_id = tmp;
	} else{
		local_thread_id = tmp + 11;
	}
	KNN::static_total_threads++;
	rv = pthread_spin_unlock(&KNN::spinlock);
	if(rv != 0){
		std::cout << "problem unlocking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	return local_thread_id;
}

void KNN::affinity(){
	//21 std::cout << "affinity" << std::endl;	
	int rv = pthread_spin_lock(&KNN::spinlock);
	if(rv != 0){
		std::cout << "problem locking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}	
	int loc_id = KNN::static_total_threads++;
	rv = pthread_spin_unlock(&KNN::spinlock);	
	if(rv != 0){
		std::cout << "problem unlocking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	
	cpu_set_t cpuset;	
	CPU_ZERO(&cpuset);
	/*	
	unsigned int i;
	unsigned int end;	
		
	if(loc_id%2==0){
		i = 0;
		end = KNN::static_cores;
	} else {
		i = KNN::static_cores;
		end = KNN::static_cores*2;
	}
		
	for(; i < end; i+=2){
		CPU_SET(i, &cpuset);
		CPU_SET(i+12, &cpuset);
		std::cout << i << std::endl;
		std::cout << i+12 << std::endl;
	}
	*/
	unsigned int core = loc_id % KNN::static_cores;
	CPU_SET(core, &cpuset);
	int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0){
    	std::cerr << "Error setting affinity_here" << '\n';
   	}
}

bool KNN::make_thread(){
	bool ret;	
	int rv = pthread_spin_lock(&KNN::thread_lock);
	if(rv != 0){
		std::cout << "problem locking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	if(KNN::tmp_thread_count < KNN::thread_limit){
		ret = true;
		//static_total_threads++;
		KNN::tmp_thread_count++;
		if(KNN::tmp_thread_count > KNN::max_threads){
			KNN::max_threads = KNN::tmp_thread_count;
		}
	} else{
		ret = false;
	}
	rv = pthread_spin_unlock(&KNN::thread_lock);
	if(rv != 0){
		std::cout << "problem unlocking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	return ret;
}


void KNN::decrement_tmp_threads(){
	int rv = pthread_spin_lock(&KNN::thread_lock);
	if(rv != 0){
		std::cout << "problem locking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
	KNN::tmp_thread_count--;
	rv = pthread_spin_unlock(&KNN::thread_lock);
	if(rv != 0){
		std::cout << "problem unlocking lock in thread ";
		std::cout << pthread_self() << std::endl;
	}
}

inline void KNN::set_depth_init_mutex(){
	//if (pthread_mutex_init(&this->lock, NULL) != 0) { 
	if(pthread_spin_init(&spinlock, 0) != 0){
        std::cout << "Problem initializing lock" << std::endl; 
        exit(0); 
    } 
		
	if(pthread_spin_init(&thread_lock, 0) != 0){
        std::cout << "Problem initializing lock" << std::endl; 
        exit(0); 
    }	
	
	KNN::static_cores = this->cores;
	if(this->cores < 2){
		this->static_depth = 1;
		KNN::thread_limit = 2*KNN::static_cores;
	}	
	else if(this->cores < 3){
		this->static_depth = 2;
		KNN::thread_limit = 2 + (2*KNN::static_cores);
	}
	else if(this->cores < 5){
		this->static_depth = 3;
		KNN::thread_limit = 6 + (2*KNN::static_cores);
	}
	else if (this->cores < 9){
		this->static_depth = 4;
		KNN::thread_limit = 14 + (2*KNN::static_cores);
	}
	else {
		this->static_depth = 5;
		KNN::thread_limit = 30 + (2*KNN::static_cores);
	}
	//this->static_depth = 2;
	//KNN::thread_limit = 2*this->cores; // 21
	std::cout << "thread_limit:\t\t" << KNN::thread_limit << std::endl;
	std::cout << "static_depth:\t\t" << this->static_depth << std::endl;
}

void KNN::set_socket_affinity(cpu_set_t &cpuset, int tid){
	CPU_ZERO(&cpuset);
	unsigned int cpu;	
	if(tid % 2 == 0 ){
		cpu = 0;
	}
	else {
		cpu = 1;	
	}
	while(cpu < KNN::static_cores){
		CPU_SET(cpu, &cpuset);
		CPU_SET(cpu+12, &cpuset);
		cpu += 2;
	}


	//CPU_SET(tid, &cpuset);
	int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0){
    	std::cerr << "Error setting affinity" << '\n';
   	}

}

void KNN::set_logical_affinity(cpu_set_t &cpuset, int tid){
	CPU_ZERO(&cpuset);
   	CPU_SET(tid, &cpuset);
	int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0){
      	std::cerr << "Error setting affinity" << '\n';
   	}

}

void KNN::set_core_affinity(cpu_set_t &cpuset, unsigned int tid){
	CPU_ZERO(&cpuset);
   	CPU_SET(tid, &cpuset);
	//std::cout << "tid: " << tid << " ";	
	if(tid < KNN::mod_logical_hw_cores){
		CPU_SET(tid+KNN::mod_logical_hw_cores, &cpuset);
	//	std::cout << "+ " << tid+KNN::mod_logical_hw_cores << std::endl;
	} else{
		CPU_SET(tid-KNN::mod_logical_hw_cores, &cpuset);
	//	std::cout << "- " << tid-KNN::mod_logical_hw_cores << std::endl;
	}

	int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0){
      	std::cerr << "Error setting affinity" << '\n';
   	}

}



void KNN::_print(const std::vector<float*> &v){
	std::cout << "debugPrint-------" << std::endl;
	for(unsigned long i = 0; i < v.size(); ++i){
		for(unsigned long j = 0; j < this->num_dimensions; ++j){
			std::cout << v[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;	
}


void KNN::_print_tree(Node* node, int space){
	if(node){
		space += 10;
		this->_print_tree(node->r_child, space);
		std::cout << std::endl;
		for(int i = 10; i < space; ++i){
			std::cout << " ";
		}
		if(node->num_pts == 1){
			std::cout << "leaf: ";			
			for(unsigned long i = 0; i < this->num_dimensions; ++i){
				std::cout << node->point[i] << ", ";
			}			
			std::cout << std::endl;
		}
		else{
			std::cout << node->dimension << " " << node->median << std::endl;
		}
		this->_print_tree(node->l_child, space);
	}
}

void KNN::_print_results(){
	std::cout << "_print_results() " << std::endl;	
	int count = 1;	
	for(unsigned long i = 0; i < this->num_queries; ++i){
		std::cout << "query: ";
		for(unsigned long k = 0; k < this->num_dimensions; ++k){	
				std::cout << this->query_pts[i][k] << ", ";
		}
		std::cout << std::endl;		
		for(long j = (this->k - 1); j >= 0; --j){
			std::cout << count << " ";	
			++count;		
			for(unsigned long k = 0; k < this->num_dimensions; ++k){	
				std::cout << this->results[i][j].point[k] << ", ";
			}
			std::cout << std::endl;

		}
		count = 1;
	}
}

void KNN::_check_output(){
	std::cout << "_check_output()" << std::endl;	
	int fd = open(this->result_f_name, O_RDONLY); 
	if(fd < 0){
		int en = errno;
        std::fprintf(stderr, "Couldn't open %s: %s\n", this->result_f_name,
																 strerror(en));
        exit(2);
	}
	struct stat sb;
    int rv = fstat(fd, &sb); assert(rv == 0);	
	size_t num_floats = this->num_queries * this->k * this->num_dimensions;
	size_t file_size = 56 + (4 * num_floats);
	assert(file_size == (size_t)sb.st_size);
	void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE|MAP_POPULATE, 
																		fd, 0);
	if (vp == MAP_FAILED) {
		close(fd);     	
		int en = errno;
    	fprintf(stderr, "mmap() failed: %s\n", strerror(en));
    	exit(3);
    }
	rv=madvise(vp, sb.st_size, MADV_SEQUENTIAL|MADV_WILLNEED); assert(rv == 0);
	rv = close(fd); assert(rv == 0);
	unsigned long idx = 0;
	char * char_ptr = (char*) vp;
	char * _res_str = strndup(&(char_ptr[idx]), 8); ++idx;
	unsigned long* ul_ptr = (unsigned long*) vp;
	unsigned long _tr_id = ul_ptr[idx]; idx++;
	unsigned long _q_id = ul_ptr[idx]; idx++;
	unsigned long _res_id = ul_ptr[idx]; idx++;
	unsigned long _num_q = ul_ptr[idx]; idx++;
	unsigned long _num_dim = ul_ptr[idx]; idx++;
	unsigned long _k = ul_ptr[idx]; idx++;
	float* fl_ptr = (float*) vp; idx*=2;// 32bitfloat-> 2*idx of 64bitlong
	
	float*** q_k_d = new float**[_num_q];
	for(unsigned long i = 0; i < _num_q; ++i){
		q_k_d[i] = new float*[_k];
		for(unsigned long j = 0; j < _k; ++j){
			q_k_d[i][j] = new float[_num_dim];
			
		}
	}
	for(unsigned long i = 0; i < _num_q; ++i){
		for(unsigned long j = 0; j < _k; ++j){
			for(unsigned long k = 0; k < _num_dim; ++k){
				q_k_d[i][j][k] = fl_ptr[idx]; ++idx;
			}
		}
	}
	assert(!strcmp(_res_str, "RESULT"));
	assert(_tr_id == this->train_id);
	assert(_q_id == this->query_id);
	assert(_res_id == this->res_id);
	assert(_num_q == this->num_queries);
	assert(_num_dim == this->num_dimensions);
	assert(_k == this->k);
	long des_idx;
	for(unsigned long i = 0; i < _num_q; ++i){
		des_idx = _k-1;		
		for(unsigned long j = 0; j < _k; ++j){
			for(unsigned long k = 0; k < _num_dim; ++k){
				assert(q_k_d[i][j][k] == this->results[i][des_idx].point[k]);
			}
			des_idx--;	
		}
	}
	

	if(munmap(vp, sb.st_size) == -1){
		std::cout << "Error on munmap in check_output" << std::endl;
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);
	
	for(unsigned long i = 0; i < _num_q; ++i){
		for(unsigned long j = 0; j < _k; ++j){
			delete[] q_k_d[i][j];
		}
		delete[] q_k_d[i];
	}
	delete[] q_k_d;
	free(_res_str);
}

//-----------------------------------------------------------------
//----------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------

void KNN::_test(){
	std::cout << "test()...cmp to brute force" << std::endl;
	
	//nearest_point* arr = new nearest_point[this->num_training_pts];
	std::vector<std::vector<nearest_point>> arr;
	arr.reserve(this->num_queries);
	
	arr.reserve(this->num_training_pts);
	for(unsigned long j = 0; j < this->num_queries; ++j){	
		std::vector<nearest_point> tmp;
		tmp.reserve(this->num_training_pts);
		for(unsigned long i = 0; i < this->num_training_pts; ++i){		
			tmp.push_back(nearest_point{this->train_pts[i], 
						this->distance(this->query_pts[j], this->train_pts[i]),
						nullptr, nullptr});
		}
		arr.push_back(tmp);
	}
	/*
	std::cout << "before sort" << std::endl;
	for(unsigned long i = 0; i < this->num_training_pts; ++i){
		std::cout << arr[i].distance << " ";
		for(unsigned long j = 0; j < this->num_dimensions; ++j){
			std::cout << arr[i].point[j] << ",";
		}
		std::cout << std::endl;
	}
	*/

	auto cmp = [](const nearest_point &p1, const nearest_point &p2){
		return p1.distance < p2.distance;
	};
	for(unsigned long i = 0; i < this->num_queries; ++i){
		std::sort((arr[i]).begin(), (arr[i]).end(), cmp);
	}
	long des_idx;
	for(unsigned long z = 0; z < this->num_queries; ++z){
		//des_idx = k - 1;	
		des_idx = this->num_found - 1;	
				
		//std::cout << "-----------------------------------" << std::endl;		
		//std::cout << "full" << std::endl;
		for(unsigned long i = 0; i < this->num_training_pts; ++i){
			//std::cout << arr[z][i].distance << " ";
			for(unsigned long j = 0; j < this->num_dimensions; ++j){
				//std::cout << arr[z][i].point[j] << ", ";
			}
			//std::cout << std::endl;
		}
		

		/*
		std::cout << "query" << std::endl;	
		for(unsigned long i = 0; i < this->num_dimensions; ++i){
			std::cout << this->query_pts[z][i] << ", ";
		}
		std::cout << std::endl;	
		std::cout << "result" << std::endl;
		*/
		for(unsigned long i = 0; i < this->num_found; ++i){
		//for(unsigned long i = 0; i < this->k; ++i){
			//std::cout << arr[z][i].distance << " ";
			for(unsigned long j = 0; j < this->num_dimensions; ++j){
				//std::cout << arr[z][i].point[j] << ", ";
				assert(arr[z][i].point[j] == this->results[z][des_idx].point[j]);
			}
			des_idx--;
			//std::cout << std::endl;
		}
		//std::cout << "-----------------------------------" << std::endl;	
	}
}

int KNN::compare_by_dimension(const void* o1, const void* o2, void *thunk){
	//std::cout << "compare" << std:: endl;	
	const unsigned long idx = *(const unsigned long*)thunk;
	//std::cout << idx << std::endl;
	const float* d1 = *(const float**)o1;
	const float* d2 = *(const float**)o2;
	//std::cout << ((*d1)[idx]) << std::endl;
	//std::cout << ((*d2)[idx]) << std::endl;
	//double oo1 = 
	if((d1[idx]) < (d2[idx])) return -1;
	if((d1[idx]) > (d2[idx])) return 1;
	return 0;
}








