#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define ANSI_COLOR_GRAY    "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

#define ANSI_COLOR_RESET   "\x1b[0m"

#define TERM_CLEAR() printf("\033[H\033[J")
#define TERM_GOTOXY(x,y) printf("\033[%d;%dH", (y), (x))

void print_progress(pid_t localTid, size_t value) {
        pid_t tid = syscall(__NR_gettid);

        TERM_GOTOXY(0,localTid+1);

	char prefix[256];
        size_t bound = 100;
        sprintf(prefix, "%d: %ld (ns) \t[", tid, value);
	const char suffix[] = "]";
	const size_t prefix_length = strlen(prefix);
	const size_t suffix_length = sizeof(suffix) - 1;
	char *buffer = calloc(bound + prefix_length + suffix_length + 1, 1);
	size_t i = 0;

	strcpy(buffer, prefix);
	for (; i < bound; ++i)
	{
	    buffer[prefix_length + i] = i < value/10000 ? '#' : ' ';
	}
	strcpy(&buffer[prefix_length + i], suffix);
        
        if (!(localTid % 7)) 
            printf(ANSI_COLOR_WHITE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 6)) 
            printf(ANSI_COLOR_BLUE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 5)) 
            printf(ANSI_COLOR_RED "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 4)) 
            printf(ANSI_COLOR_GREEN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 3)) 
            printf(ANSI_COLOR_CYAN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 2)) 
            printf(ANSI_COLOR_YELLOW "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 1)) 
            printf(ANSI_COLOR_MAGENTA "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else
            printf("\b%c[2K\r%s\n", 27, buffer);

	fflush(stdout);
	free(buffer);
}

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
