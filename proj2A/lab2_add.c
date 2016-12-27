#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>


//initiailization
static long long counter = 0;
static int thread_number = 1;
static int iteration_number = 1;
static int yield = 0;
static int sync = 0;
static char name[12]="add-none";

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
volatile int spin_lock = 0; 

typedef enum {
	NO_LOCK_OPTION, 
	PTHREAD_MUTEX_OPTION,
	SPIN_LOCK_OPTION,
	COMPARE_SWAP_OPTION
} lock_type; 

lock_type LOCK_OPTION = NO_LOCK_OPTION;

//four ways to add----------------------
//add
void add(long long *pointer, long long value) {
	long long sum = *pointer + value; 
	if(yield)
		pthread_yield();
	*pointer = sum;
}

//add with mutex locks
void add_mutex(long long *pointer, long long value) {
	pthread_mutex_lock(&mutex_lock);
	long long sum = *pointer + value;
	if(yield)
		pthread_yield(); 
	*pointer = sum;
	pthread_mutex_unlock(&mutex_lock);
}

//spin
void add_TestAndSet(long long *pointer, long long value) 
{
	while(__sync_lock_test_and_set (&spin_lock, 1))
		;
	long long sum = *pointer + value;
	if(yield)
		pthread_yield(); 
	*pointer = sum;
	__sync_lock_release (&spin_lock);
}

//compare and swap
void add_CompareAndSwap(long long *pointer, long long value)
{
	long long prev = 0;
	long long sum;
	do{
		prev = counter;
		sum = prev + value;
		if(yield)
			pthread_yield();
	}
	while(__sync_val_compare_and_swap (&counter, prev, sum)!=prev);
}

//execution of each thread---------------------------------
void* my_thread(void* ptr)
{
	for(int i = 0; i < iteration_number; i++)
	{
		switch(LOCK_OPTION){
			case NO_LOCK_OPTION:
				add(&counter,1);
				break;
			case PTHREAD_MUTEX_OPTION:
				add_mutex(&counter,1);
				break;
			case SPIN_LOCK_OPTION:
				add_TestAndSet(&counter,1);
				break;
			case COMPARE_SWAP_OPTION:
				add_CompareAndSwap(&counter,1);
				break;
			default:
				break;
		}
	}
	for(int j = 0; j < iteration_number; j++)
	{
		switch(LOCK_OPTION){
			case NO_LOCK_OPTION:
				add(&counter,-1);
				break;
			case PTHREAD_MUTEX_OPTION:
				add_mutex(&counter,-1);
				break;
			case SPIN_LOCK_OPTION:
				add_TestAndSet(&counter,-1);
				break;
			case COMPARE_SWAP_OPTION:
				add_CompareAndSwap(&counter,-1);
				break;
			default:
				break;
		}
	}
	return NULL;
}

//main---------------------------------
int main(int argc, char *argv[]){
	//declarations
	struct timespec start_time, end_time;
  	struct option long_options[]=
  	{
    	{"threads",required_argument,NULL,'t'},
    	{"iterations",required_argument,NULL,'i'},
    	{"yield",no_argument,NULL,'y'},
    	{"sync",required_argument,NULL,'s'}
  	};

	//get input
  	int ret = 0;
  	while(1){
    	ret = getopt_long(argc,argv,"t:i:ys:",long_options,NULL);
    	if(ret == -1)
      		break;
    	switch(ret){
	    	case 't':
	    		thread_number = atoi(optarg);
	    		break;
		    case 'i':
		    	iteration_number = atoi(optarg);
		    	break;
		    case 'y':
	    		yield = 1;
	    		break;
		    case 's':
		    	sync = 1;
		    	if(*optarg == 'm'){
		    		LOCK_OPTION = PTHREAD_MUTEX_OPTION;
		    	}
		    	if(*optarg == 's'){
		    		LOCK_OPTION = SPIN_LOCK_OPTION;
		    	}
		    	if(*optarg == 'c'){
		    		LOCK_OPTION = COMPARE_SWAP_OPTION;
		    	}
		    	break;
		    default:
		    	break;
    	}
  	}

  	//get time
  	clock_gettime(CLOCK_MONOTONIC,&start_time);

  	pthread_t thread[thread_number];
  	for(int i = 0; i < thread_number; i++)
  	{
		if(pthread_create(&thread[i],NULL,my_thread,NULL))
		{
			fprintf(stderr, "Error: cannot create new thread.\n");
			exit(1);
		}
  	}
  	for(int i = 0; i < thread_number; i++)
  	{
  		pthread_join(thread[i], NULL);
  	}

  	clock_gettime(CLOCK_MONOTONIC,&end_time);

  	long long operation_number = thread_number*iteration_number*2;
  	long long total_time = (end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
  	long long average_time = total_time/operation_number;

  	//print
  	if(yield){
  		strcpy(name, "add-yield");
  	}
  	if(sync)
  	{
	       	if(yield)
  		{
	  		if(LOCK_OPTION==PTHREAD_MUTEX_OPTION)
	  			strcpy(name, "add-yield-m");
	  		if(LOCK_OPTION==SPIN_LOCK_OPTION)
	  			strcpy(name, "add-yield-s");
	   		if(LOCK_OPTION==COMPARE_SWAP_OPTION)
	   			strcpy(name, "add-yield-c");
	  	}
	  	else
	  	{
	  		if(LOCK_OPTION==PTHREAD_MUTEX_OPTION)
	  			strcpy(name, "add-m");
	  		if(LOCK_OPTION==SPIN_LOCK_OPTION)
	  			strcpy(name, "add-s");
	   		if(LOCK_OPTION==COMPARE_SWAP_OPTION)
	   			strcpy(name, "add-c");
	  	}
  	}

  	printf("%s,", name);
  	printf("%d,%d,%lld,%lld,%lld,%lld\n", thread_number, iteration_number, operation_number, total_time,average_time,counter);


  exit(0);
}
