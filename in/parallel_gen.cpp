#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <thread>
#include <assert.h>


struct Info {
	unsigned int id;
	unsigned long start;
	unsigned long end;
};

static void* generate(void* vp);

int main(){
	unsigned int cores = std::thread::hardware_concurrency();	
	std::cout << cores << std::endl;
	/*	
	if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init failed\n"); 
        return 1; 
    } 	
	*/
	/*
	char str [200];
	FILE* in = popen("pwd", "r");

	fscanf(in, "%s", str);

	pclose(in);
	
	std::cout << str << std::endl;
	*/
	Info * info_arr = new Info[cores];
	unsigned long start = 20000;
	unsigned long s = start;	
	unsigned long interval = 1;	
	for(unsigned int i = 0; i < cores; i++){
		info_arr[i] = Info{i, start, start+interval};
		start+=interval;
	}
	
	int rv;
	pthread_t* tids = new pthread_t[cores];
	
	for(long i = 0; i < cores; i++){
		rv = pthread_create(&(tids[i]), NULL, generate, (void*)&(info_arr[i]) );
		assert(rv == 0); 
	}
	for(long i = 0; i < cores; i++){
		rv = pthread_join(tids[i], NULL);
	}
	std::cout << "start " << s << std::endl;	
	std::cout << "end " << start << std::endl;
	return 0;
}

static void* generate(void* vp){
	Info* info = (Info*)vp;
	// set affinity
	int rc;
	cpu_set_t cpuset;
   	CPU_ZERO(&cpuset);
   	CPU_SET(info->id, &cpuset);
	rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0){
      	std::cerr << "Error setting affinity" << '\n';
   	}
	
	//std::string id = std::to_string(info->id);
	//std::string command = "mkdir " + id;
	//std::cout << command << std::endl;
	//rc = system(command.c_str()); assert(!rc);
	//command = "cp query_file " + id;
	//rc = system(command.c_str());  assert(!rc);
	//command = "cp training_data " + id;
	//rc = system(command.c_str());  assert(!rc);
	//command = "cp pgen.sh " + id;
	//rc = system(command.c_str());  assert(!rc);
	//command = "chmod +x " +id + "/pgen.sh";
	//rc = system(command.c_str());  assert(!rc);
	//command = "cd " + id;
	//rc = system(command.c_str());  assert(!rc);
	//rc = system("pwd");  assert(!rc);

		
	std::string command =  "./pgen.sh " + std::to_string(info->start) + " " + 
									std::to_string(info->end);
	rc = system(command.c_str());  assert(!rc);
	

	delete info;
	return nullptr;
}
