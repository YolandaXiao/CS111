#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include "SortedList.h"


//initiailization
static int thread_number = 1;
static int iteration_number = 1;

static int yield = 0;
static int sync = 0;
static char* yield_option;
static char sync_option;
int opt_yield = 0;

static char name[12]="list";
static char yield_name[6]="-";
static char sync_name[6]="-none";

static pthread_t *thread;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
int spin_lock = 0; 

static SortedList_t head; 
static SortedListElement_t *element; 
int test_flag = 0;


//execution of each thread---------------------------------
void* my_thread(void* num_thread)
{
	// int *conver = (int *) num_thread;
	// int start = (*conver) * iteration_number;
	// int end = start + iteration_number;

	SortedListElement_t* start = &element[*((int*)num_thread) * iteration_number];

	//insert
	for(int i = 0; i < iteration_number; i++)
	{
		switch(sync_option){
			case 'm':
				pthread_mutex_lock(&mutex_lock);
				SortedList_insert(&head, &start[i]);
				pthread_mutex_unlock(&mutex_lock);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				SortedList_insert(&head, &start[i]);
				__sync_lock_release(&spin_lock);
				break;
			default:
				SortedList_insert(&head, &start[i]);
				break;;
		}
	}
	//length
	switch(sync_option){
		case 'm':
			pthread_mutex_lock(&mutex_lock);
			SortedList_length(&head);
			pthread_mutex_unlock(&mutex_lock);
			break;
		case 's':
			while (__sync_lock_test_and_set(&spin_lock, 1));
			SortedList_length(&head);
			__sync_lock_release(&spin_lock);
			break;
		default:
			SortedList_length(&head);
			break;;
		}
	//lookup & delete
	SortedListElement_t *matched_elem;
	for(int j = 0; j < iteration_number; j++)
	{
		switch(sync_option){
			case 'm':
				pthread_mutex_lock(&mutex_lock);
				matched_elem = SortedList_lookup(&head, start[j].key);
				SortedList_delete(matched_elem);
				pthread_mutex_unlock(&mutex_lock);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				matched_elem = SortedList_lookup(&head, start[j].key);
				SortedList_delete(matched_elem);
				__sync_lock_release(&spin_lock);
				break;
			default:
				matched_elem = SortedList_lookup(&head, start[j].key);
				SortedList_delete(matched_elem);
				break;;
		}
	}
	free(num_thread);
  	pthread_exit(NULL);
}

//main---------------------------------
int main(int argc, char *argv[]){
	//declarations
	struct timespec start_time, end_time;
  	struct option long_options[]=
  	{
    	{"threads",required_argument,NULL,'t'},
    	{"iterations",required_argument,NULL,'i'},
    	{"yield",required_argument,NULL,'y'},
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
	    		yield_option = optarg;
	    		break;
		    case 's':
		    	sync = 1;
		    	sync_option = *optarg;
		    	break;
		    default:
		    	abort();
    	}
  	}

  	if(yield)
  	{
  		for(int i = 0; i < strlen(yield_option); i++)
  		{
  			if(yield_option[i] == 'i')
  			{
  				opt_yield |= INSERT_YIELD; 
  				strcat(yield_name,"i");
  			}
  			else if(yield_option[i] == 'd')
  			{
  				opt_yield |= DELETE_YIELD;
  				strcat(yield_name,"d");
  			}
  			else if(yield_option[i] == 'l')
  			{
  				opt_yield |= LOOKUP_YIELD;
  				strcat(yield_name,"l");
  			}
  			else
  			{
			  fprintf(stderr, "ERROR: non-valid input for yield.\n");
			  exit(1);
  			}

  		}
  	}
	else
	  strcat(yield_name,"none");

  	if (sync && (sync_option != 'm' && sync_option != 's'))
  	{
	    fprintf(stderr, "ERROR: non-valid input for sync.\n");
	    exit(1);
  	} 

  	//initialize threads
  	int calculation_num = thread_number*iteration_number;
  	const char* random_key = "0123456789abcdefghijklmnopqrstuvwxyz";
  	thread = (pthread_t *)malloc(sizeof(pthread_t) * thread_number);
  	element = (SortedListElement_t *)malloc(sizeof(SortedListElement_t) * calculation_num);

  	//initialize list
  	head.key = NULL;
  	head.prev = &head;
  	head.next = &head;

  	//create keys
  	srand(time(NULL));
  	int key_size = 6;
  	for(int i = 0; i < calculation_num; i++)
  	{
  		char *temp = (char *)malloc(sizeof(char) * key_size);
  		for(int j = 0; j < key_size-1; j++)
  		{
  			temp[j] = random_key[rand()%(sizeof(random_key)-1)];
  		}
  		temp[key_size-1] = '\0';
  		element[i].key = temp;
  	}


  	//start time
  	clock_gettime(CLOCK_MONOTONIC,&start_time);

  	for(int i = 0; i < thread_number; i++)
  	{
  		int *temp = (int *)malloc(sizeof(int*));
  		*temp = i;
		if(pthread_create(&thread[i],NULL,my_thread,(void *)temp))
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

  	long long operation_number = thread_number*iteration_number*3;
  	long long total_time = (end_time.tv_sec - start_time.tv_sec)*1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
  	long long average_time = total_time/operation_number;


  	//print
  	if(sync)
  	{
	  	if(sync_option=='m')
	  		strcpy(sync_name, "-m");
	 	if(sync_option=='s')
	  		strcpy(sync_name, "-s");
  	}
  	strcat(name, yield_name);
  	strcat(name, sync_name);

  	//cleanup
	for (int i = 0; i < thread_number * iteration_number; i++)
	    free((void*)element[i].key);

	free(thread);
    free(element);

    //print
  	printf("%s,", name);
  	printf("%d,%d,", thread_number, iteration_number);
  	//check length
  	if(SortedList_length(&head)!=0){
  		test_flag = 1;
  		fprintf(stderr, "ERROR: list length is %d\n", SortedList_length(&head));
  	}
  	else
  		printf("%d,", SortedList_length(&head));
  	printf("%lld,%lld,%lld\n", operation_number, total_time,average_time);

  	if(test_flag==0)
  		exit(0);
  	else
  		exit(1);
}

