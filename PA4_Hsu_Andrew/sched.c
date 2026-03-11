#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "print_progress.c"

void* arraySum(void* inputData);
typedef struct _thread_data_t {
	int localTid;
	const int *data;
	int numVals;
	pthread_mutex_t *lock;
	long long int *totalSum;
} thread_data_t;


void* arraySum(void* inputPtr){
	thread_data_t* threadPtr = (thread_data_t*)inputPtr;
	while(1){
		long long int threadSum = 0;
		long latency_max = 0;
		for(int i = 0; i < threadPtr->numVals; i++){
			long latency;
			struct timespec start;
			struct timespec end;
			
			clock_gettime(CLOCK_REALTIME, &start); 
			threadSum += threadPtr->data[i];
			clock_gettime(CLOCK_REALTIME, &end);
			
			latency = (end.tv_sec - start.tv_sec) * 1000000000.0;
			latency += (end.tv_nsec - start.tv_nsec) ;
			
			if(latency > latency_max){
				latency_max = latency;
			}
		}	
		pthread_mutex_lock(threadPtr->lock);
		*(threadPtr->totalSum) += threadSum;
		pthread_mutex_unlock(threadPtr->lock);
		
		print_progress(threadPtr->localTid, latency_max);
	}
}

int main(int argc, char* argv[]){
	pthread_mutex_t mutex;
	long long int totalSum = 0;
	int* intArray;
	if(argc < 2){
		printf("There aren't enough parameters.\n");
		return -1;
	}
	
	printf("Number of Processors: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
	thread_data_t threadArray[atoi(argv[1])];
	pthread_t pThreadArray[atoi(argv[1])];
	intArray = (int*)malloc(2000000 * sizeof(int));	
	pthread_mutex_init(&mutex, NULL);
	for(int i = 0; i < atoi(argv[1]); i++){
		threadArray[i].localTid = i;
		threadArray[i].data = intArray;
		threadArray[i].numVals = 2000000;
		threadArray[i].lock = &mutex;
		threadArray[i].totalSum = &totalSum;
	}
	for(int i = 0; i < atoi(argv[1]); i++){
		void* voidThreadArray = &threadArray[i];
		pthread_create(&pThreadArray[i], NULL, arraySum, voidThreadArray);
	}
	for(int i = 0; i < atoi(argv[1]); i++){
		pthread_join(pThreadArray[i], NULL);
	}
	return 0;
}
