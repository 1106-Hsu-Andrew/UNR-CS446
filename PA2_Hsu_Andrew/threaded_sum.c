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
    thread_data_t* threadPtr = (thread_data_t*)inputPtr;

    long long int threadSum = 0;
    for(int i = threadPtr->startInd; i < threadPtr->endInd; i++){
        threadSum += threadPtr->data[i];
    }

    pthread_mutex_lock(threadPtr->lock);
    *(threadPtr->totalSum) += threadSum;
    pthread_mutex_unlock(threadPtr->lock);
}

int readFile(char fileName[], int fileContents[]){
    int successfulReads = 0;
    FILE* fptr;

    fptr = fopen(fileName, "r");
    if(fptr == NULL){
        printf("File not found...\n");
        return -1;
    }
    
    while(fscanf(fptr, "%d", &fileContents[successfulReads]) == 1){
        successfulReads++;
    }
    fclose(fptr);
    return successfulReads;
    
}

int main(int argc, char* argv[]){
    pthread_mutex_t mutex;
    struct timeval startTime;
    int fileContents[1000000];
    long long int totalSum = 0;
    int totalThreads = atoi(argv[2]);

    if(argc != 3){
        printf("There aren't enough parameters.\n");
        return -1;
    }
    int fileLen = readFile(argv[1], fileContents);

    if(totalThreads > fileLen){
        printf("Too many threads requested.\n");
        return -1;
    }
    gettimeofday(&startTime, NULL);
    pthread_mutex_init(&mutex, NULL);

    thread_data_t threadData[totalThreads];
    for(int i = 0; i < totalThreads; i++){
        threadData[i].data = fileContents;
        threadData[i].startInd = (fileLen / totalThreads) * i;

        if(i == totalThreads - 1){
            threadData[i].endInd = fileLen;
        }
        else{
            threadData[i].endInd = (fileLen / totalThreads) * (i + 1);
        }
        threadData[i].lock = &mutex;
        threadData[i].totalSum = &totalSum;
    }

    pthread_t threadArray[totalThreads];
    struct timeval endTime;
    double elapsedTime;
    for(int i = 0; i < totalThreads; i++){
        void* voidThreadData = &threadData[i];
        pthread_create(&threadArray[i], NULL, arraySum, voidThreadData);
    }

    for(int i = 0; i < totalThreads; i++){
        pthread_join(threadArray[i], NULL);
    }
    gettimeofday(&endTime, NULL);
    elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000.0;
    elapsedTime += (endTime.tv_usec - startTime.tv_usec) / 1000.0;

    printf("Total Sum: %lld\nTotal Execution Time: %f ms\n", totalSum, elapsedTime);
    return 0;
}