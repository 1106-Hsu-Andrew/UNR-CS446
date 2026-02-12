#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

int readFile(char[], int[]);
void* arraySum(void*);

typedef struct _thread_data_t{
    const int* data;
    int startInd;
    int endInd;
    pthread_mutex_t* lock;
    long long int* totalSum;
} thread_data_t;

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

void* arraySum(void* inputPtr){

}

int main(int argc, char* argv[]){
    int fileContents[1000000];

    if(argc != 3){
        printf("There aren't enough parameters.");
        return -1;
    }

    int fileLen = readFile(argv[1], fileContents);
    if(argv[2] > fileLen){
        printf("Too many threads requested.");
        return -1;
    }
}