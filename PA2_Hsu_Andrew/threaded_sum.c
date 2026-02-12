#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

void* arraySum(void*);
int readFile(char[], int[]);

typedef struct _thread_data_t{
    const int* data;
    int startInd;
    int endInd;
    pthread_mutex_t* lock;
    long long int* totalSum;
} thread_data_t;

void* arraySum(void* inputPtr){

}

int readFile(char fileName[], int fileContents[]){
    int successfulReads = 0;
    FILE* fptr;

    fptr = fopen(fileName, "r");
    if(fptr == NULL){
        printf("File not found...");
        return -1;
    }
    
    while(fscanf(fptr, "%d", &fileContents[successfulReads]) == 1){
        successfulReads++;
    }
    fclose(fptr);
    return successfulReads;
    
}

int main(int argc, char* argv[]){
    pthread_mutex_t* mutex;
    struct timeval currTime;
    int fileContents[1000000];
    long long int totalSum = 0;
    int totalThreads = atoi(argv[2])

    if(argc != 3){
        printf("There aren't enough parameters.");
        return -1;
    }
    int fileLen = readFile(argv[1], fileContents);

    if(totalThreads > fileLen){
        printf("Too many threads requested.");
        return -1;
    }
    gettimeofday(&currTime, NULL);
    pthread_mutex_init(mutex, NULL);

    thread_data_t threadData[fileLen];
    for(int i = 0; i < fileLen - 1; i++){
        threadData[i].data = fileContents;
        threadData[i].startInd = (fileLen / totalThreads) * i;
        threadData[i].endInd = (fileLen / totalThreads) * (i + 1);
        threadData[i].lock = threadData[i - 1];
    }
}