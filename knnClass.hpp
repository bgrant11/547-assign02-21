#ifndef KNN_CLASS_H
#define KNN_CLASS_H

#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <vector>
#include <cmath>

#include <algorithm>
#include <array>
#include <queue>
#include <chrono>
#include <thread>
#include <atomic>
#include <stdint.h>
#include <float.h>

#include <sched.h>

#include <pthread.h>

//#include <math.h>

/*
class KNN;
struct nearest_point;
struct priority_compare;

typedef std::priority_queue<nearest_point, std::vector<nearest_point>,
				priority_compare> result_queue;

*/
struct train_info{
	char * file_type;
	unsigned long id;
	unsigned long num_pts;
	unsigned long num_dimensions;
	float ** points;
};	

struct query_info{
	char * file_type;
	unsigned long id;
	unsigned long num_queries;
	unsigned long num_dimensions;
	unsigned long k;
	float ** points;
	
};

struct a_result{
	unsigned long k;	
	float * query;
	float ** kn;
};

struct results_info{
	char* file_type;
	unsigned long train_id;
	unsigned long query_id;
	unsigned long id;
	unsigned long num_queries;
	unsigned long num_dimensions;
	unsigned long k;
	a_result * results;
};

struct Range {
	const ssize_t begin, end;
};

struct Info {
	const std::vector<float*> &a;
	const Range r;
	const unsigned int depth;
	const unsigned long dimension;
	//const unsigned long thread_id;
};





class KNN{
	class Node{
		public:		
			Node(unsigned long dimension, float median, unsigned long num_pts) 
				:dimension{dimension}, 
				 median{median}, 
				 num_pts{num_pts},
				 l_child{nullptr}, 
				 r_child{nullptr}, 
				 parent{nullptr},
				 point{nullptr} 
			{}
					
		
			// insert
			// build tree			
			unsigned long dimension;
			float median; 
			unsigned long num_pts;
			Node* l_child;
			Node* r_child;		
			Node* parent;	
			float* point;
	};
	public:

		
	
	
		struct node_info{
			std::vector<float*> &a;
			const unsigned long depth;
			const unsigned long dimension;	
			Node* &n;	
			Node* &parent;				
			KNN* obj;
			bool is_thread;
		};
		struct nearest_point {
			float* point;
			float distance;
			nearest_point* next;
			nearest_point* prev;
		};
		
		struct priority_compare {
			bool operator()(const nearest_point a, const nearest_point b){
				return a.distance < b.distance;
			}
		};		
		
		typedef std::priority_queue<nearest_point, std::vector<nearest_point>,
				priority_compare> result_queue;	
		
		struct query_args {
			const float* query_point;
			unsigned long &num_found;			
			Node* &node;
			KNN* obj;
			nearest_point* head;
			nearest_point* tail;
			//std::priority_queue<nearest_point, std::vector<nearest_point>,
			//	priority_compare>* queue;
			result_queue* queue;	
			std::vector<float>& closest_dim_pt;
			//float* closest_dim_pt;
			float& closest_dim_pt_dist;
		};
		
		struct work_info{
			unsigned int work_core;
			std::vector<query_args*> &work;
		};
		
		struct Info2 {
			const std::vector<std::vector<nearest_point>> &a;
			const Range r;
			const unsigned int depth;
			const unsigned long dimension;
			//const unsigned long thread_id;
		};

		KNN(unsigned int cores, unsigned long num_training_pts, 
			unsigned long num_dimensions, unsigned long num_queries, 
			unsigned long k, float** train, float** query, 
			unsigned long train_id, unsigned long query_id, char* result_f_name)  
				:	cores{cores}, num_training_pts{num_training_pts}, 
					num_dimensions{num_dimensions},num_queries{num_queries},
					k{k}, train_pts{train}, query_pts{query}, 
					train_id{train_id}, query_id{query_id},
					result_f_name{strdup(result_f_name)}, root{nullptr},
					result_head{nullptr}, result_tail{nullptr}, num_found{0},
					results{nullptr} {}
		~KNN();	
		void delete_nodes(Node* node);	
		void make_tree();
		static void* populate_nodes(void* vp);		
		void do_queries();		
		static void* query_nodes(void* vp);	
		float distance(const float* query, const float* point);
		float distance(const float* query, const std::vector<float> point);
		float square(float a);
		
		void cpy_to_vect(std::vector<float*> &v);		
		static void* wrapper(void* vp);
		
		static std::vector<float*> 
		 	merge_sort(const std::vector<float*> &a, 
					const Range &r, 
					const unsigned int depth,
					const unsigned long dimension, bool is_thread);
		
		float get_median_and_split(const std::vector<float*> &a,
				const unsigned long &dimension);

		void split(const std::vector<float*> &a,
				std::vector<float*> &left,
				std::vector<float*> &right, 
				const unsigned long &dimension,
				const float median);

		unsigned long cycle_dimensions(unsigned long dim);
		void results_to_file();
		unsigned long urandom();
		static int get_thread_id();
		void set_depth_init_mutex();
		static void set_socket_affinity(cpu_set_t &cpuset, int tid);
		static void set_logical_affinity(cpu_set_t &cpuset, int tid);
		static void set_core_affinity(cpu_set_t &cpuset, unsigned int tid);
		static void decrement_tmp_threads();
		static bool make_thread();
		static void affinity();

		void _print(const std::vector<float*> &v);
	
		void _print_tree(Node* node, int space);			
		void _print_results();	
		void _check_output();	
		void _test();
		
		static int compare_by_dimension(const void* o1, const void* o2, 
																void *thunk);


		std::vector<std::vector<nearest_point>>
			merge_sort_res(const std::vector<std::vector<nearest_point>> 
			&a, const Range &r, const unsigned int depth,
			const unsigned long dimension);
		
		void* wrapper_res(void* vp);
		
		static void* adhoc_worker(void* vp);
		
		const unsigned int cores;									
		unsigned long num_training_pts;
		unsigned long num_dimensions;
		unsigned long num_queries;
		unsigned long k;
		float** train_pts;
		float** query_pts;
		const unsigned long train_id;
		const unsigned long query_id;
		unsigned long res_id;
		char* result_f_name;
		Node* root;
		nearest_point* result_head;
		nearest_point* result_tail;
		unsigned long num_found;
		std::vector<nearest_point>* results;

		//static pthread_mutex_t lock;
		static pthread_spinlock_t spinlock;		
		
		static pthread_spinlock_t thread_lock;
		

		static unsigned int global_thread_id;
		static unsigned int mod_logical_hw_cores;
		static unsigned int static_depth;
		static unsigned int tmp_thread_count;
		static unsigned int max_threads;
		static unsigned int static_cores;
		static unsigned int static_total_threads;
		static unsigned int thread_limit;
	
		std::chrono::time_point<std::chrono::steady_clock> tree_start;
		std::chrono::time_point<std::chrono::steady_clock> tree_end;
		//static std::atomic<unsigned long> thread_count;
		
		

};



#endif
