#include "knnClass.hpp"
#include <dirent.h>

unsigned int urandom();
bool file_exists(std::string& filename);
char* get_file(char* train_file_name);
void train_info_ctor(train_info* t, void* data_in);
void query_info_ctor(query_info* q, void* query_in);
bool check_info(train_info* t, query_info* q);
void dealloc_info(train_info* t, query_info* q);


void set_result_filename(std::string& result, char* in); // not needed
void _see_args(unsigned int& cores, std::string& data, std::string& query,
														std::string& result);

int main(int argc, char **argv){
	
	//std::cout << "cores\t\t\t" << std::thread::hardware_concurrency() << std::endl;

	if(argc != 5){
		std::cout << "invoke as ./k-nn <cores> <train_file> \
						<query_file> result_" << std::endl;
	}
		unsigned int cores = atoi(argv[1]);
		std::string data = argv[2];
		std::string query = argv[3];
		std::string result = argv[4];
		char * c_result = argv[4];
		//printf("c_result %s\n", c_result);
		//std::cout << data << std::endl;
		//std::cout << query << std::endl;	
		
		if(file_exists(result)){
			std::cout << "result file: " << result << " already exists" 
														<< std::endl; return -1;
		}		

	
		
		auto tree_start = std::chrono::steady_clock::now();
		
		// TODO MUNMAP THESE??	
		
		void* data_in = get_file(argv[2]);
		void* query_in = get_file(argv[3]);
		
		train_info t_info;
		query_info q_info;
		train_info_ctor(&t_info, data_in);
		query_info_ctor(&q_info, query_in);
		//std::cout << "t_id " << t_info.id << std::endl;
		//std::cout << "q_id " << q_info.id << std::endl;
		if(!check_info(&t_info, &q_info)){
			std::cout << "training/query info does not match" << std::endl;
			return -1;
		}
	
		KNN knn = KNN(cores, t_info.num_pts, t_info.num_dimensions,
											q_info.num_queries, q_info.k, 
											t_info.points, q_info.points,
											t_info.id, q_info.id, c_result);
	
		knn.make_tree();
		auto tree_end = std::chrono::steady_clock::now();
		//unsigned long test = 0;
		//unsigned int result_id = urandom();
		//_see_args(cores, data, query, result);
		knn.do_queries();		
		auto query_end = std::chrono::steady_clock::now();
		
		//knn._test(); // only do after do_queries //THIS
		
		//knn._print_results();		
		
		//knn._check_output();	// THIS


		auto tree_interval = 
				std::chrono::duration_cast<std::chrono::milliseconds>
				(tree_end-tree_start);
		auto query_interval = 
				std::chrono::duration_cast<std::chrono::milliseconds>
				(query_end-tree_end);
		//knn._print_results();		
		//std::cout << "num_train " << t_info.num_pts << std::endl;
		dealloc_info(&t_info, &q_info);
		std::cout << "\t\t\t\t\t\t\tcores: " << cores << std::endl;
		std::cout << "\t\t\t\t\t\t\ttree build time:\t" << tree_interval.count();
		std::cout << std::endl;		
		
		std::cout << "\t\t\t\t\t\t\tquery time:\t\t" << query_interval.count();
		std::cout << std::endl;		
		std::cout << "total thread count:\t" << KNN::static_total_threads;
		std::cout << std::endl;
		std::cout << "tmp thread count:\t" << KNN::tmp_thread_count;
		std::cout << std::endl;		
		std::cout << "max thread count:\t" << KNN::max_threads;		
		std::cout << std::endl;
		//std::cout << "ok" << std::endl;
		std::cout << std::endl;
	return 0;
}

/*
unsigned int urandom(){
	unsigned int random = 0;
	//size_t size = sizeof(random);
	std::ifstream urandom("/dev/urandom");
	if(!urandom){
		std::cout << "/dev/urandom Failed" << std::endl;
		exit(1);
	}
	urandom.read(reinterpret_cast<char*>(&random), sizeof(random));
	urandom.close();
	return random % 7000000; 		
}
*/

bool file_exists(std::string& filename){
	//std::cout << "check" << std::endl;	
	std::ifstream check(filename);
	if(check) return true;
	return false;
}


char* get_file(char* file_name){
	int fd = open(file_name, O_RDONLY); 
	if(fd < 0){
		int en = errno;
        std::fprintf(stderr, "Couldn't open %s: %s\n", file_name, strerror(en));
        exit(2);
	}
	struct stat sb;
    int rv = fstat(fd, &sb); assert(rv == 0);	
	//dest = (char*)malloc(sb.st_size);
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
	
	return (char *) vp;
	/*
	for(int i = 0; i < 8; ++i){
		std::cout << dest[i] << std::endl;
	}
	*/
	
}

void train_info_ctor(train_info* t, void* data_in){		
	unsigned long idx = 0;	
	char * char_ptr = (char*) data_in;	
	t->file_type = strndup(&(char_ptr[idx]), 8); ++idx;
	unsigned long* ul_ptr = (unsigned long*) data_in;
	t->id = ul_ptr[idx]; ++idx;
	t->num_pts = ul_ptr[idx]; ++idx;
	t->num_dimensions = ul_ptr[idx]; ++idx;
	float* fl_ptr = (float*) data_in; idx*=2;// 32bitfloat-> 2*idx of 64bitlong 
	t->points = new float*[t->num_pts];
	float* tmp_point;
	for(unsigned long i = 0; i < t->num_pts; ++i){
		//std::cout << "i " << i << std::endl;
		tmp_point = new float[t->num_dimensions];
		for(unsigned long j = 0; j < t->num_dimensions; ++j){
			//std::cout << "j " << j << std::endl;
			tmp_point[j] = fl_ptr[idx]; ++idx;
		}
		t->points[i] = tmp_point;
	}
	
	/*
	std::cout << "-----------------" << std::endl;
	std::cout << "train_info" << std::endl;
	std::cout << "file_type: " << t->file_type << std::endl;
	std::cout << "id: " << t->id << std::endl;
	std::cout << "num_pts: " << t->num_pts << std::endl;
	std::cout << "dimenstions " << t->num_dimensions << std::endl;
	std::cout << "points" << std::endl;
	for(unsigned long i = 0; i < t->num_pts; ++i){
		for(unsigned long j = 0; j < t->num_dimensions; ++j){
			std::cout << t->points[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;
	*/
}

void query_info_ctor(query_info* q, void* query_in){//TODO check retuen of new?
	unsigned long idx = 0;
	char* char_ptr = (char*) query_in;
	q->file_type = strndup(&(char_ptr[idx]), 8); ++idx;
	unsigned long* ul_ptr = (unsigned long*) query_in;
	q->id = ul_ptr[idx]; ++idx;
	q->num_queries = ul_ptr[idx]; ++idx;
	q->num_dimensions = ul_ptr[idx]; ++idx;
	q->k = ul_ptr[idx]; ++idx;
	float* fl_ptr = (float*) query_in; idx*=2;//32bitfloat-> 2*idx of 64bitlong 
	q->points = new float*[q->num_queries];
	float* tmp_point;
	for(unsigned long i = 0; i < q->num_queries; ++i){
		//std::cout << "i " << i << std::endl;
		tmp_point = new float[q->num_dimensions];
		for(unsigned long j = 0; j < q->num_dimensions; ++j){
			//std::cout << "j " << j << std::endl;
			tmp_point[j] = fl_ptr[idx]; ++idx;
		}
		q->points[i] = tmp_point;
	}
	/*	
	std::cout << "-----------------" << std::endl; 
	std::cout << "query_info" << std::endl;
	std::cout << "file_type: " << q->file_type << std::endl;
	std::cout << "id: " << q->id << std::endl;
	std::cout << "num_queries: " << q->num_queries << std::endl;
	std::cout << "dimenstions " << q->num_dimensions << std::endl;
	std::cout << "k " << q->k << std::endl;
	std::cout << "points" << std::endl;
	for(int i = 0; i < q->num_queries; ++i){
		for(int j = 0; j < q->num_dimensions; ++j){
			std::cout << q->points[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-----------------" << std::endl;
	*/
}

bool check_info(train_info* t, query_info* q){
	if(strcmp(t->file_type, "TRAINING")){
		dealloc_info(t, q);	return false;
	}
	if(strcmp(q->file_type, "QUERY")){
		dealloc_info(t, q);	return false;
	}
	if(t->num_dimensions != q->num_dimensions){
		dealloc_info(t, q);	return false;
	}
	return true;

}
void dealloc_info(train_info* t, query_info* q){
	free(t->file_type);
	free(q->file_type);	
		
	for(unsigned long i = 0; i < t->num_pts; ++i){
		delete[] t->points[i];
	}
	delete[] t->points;
	
	for(unsigned long i = 0; i < q->num_queries; ++i){
		delete[] q->points[i];
	}
	delete[] q->points;
}

/*
// not needed
void set_result_filename(std::string& result, char* in){
	result = in + std::to_string(urandom())+ ".dat";
	while(file_exists(result)){	
		result = in + std::to_string(urandom())+ ".dat";
	}
}

*/

void _see_args(unsigned int& cores, std::string& data, std::string& query,
														std::string& result){
	std::cout << "-----------------------" << std::endl;	
	std::cout << "cores: " << cores << std::endl;
	std::cout << "data: " << data << std::endl;
	std::cout << "query: " << query << std::endl;
	std::cout << "result: " << result << std::endl;
	std::cout << "-----------------------" << std::endl;
}
