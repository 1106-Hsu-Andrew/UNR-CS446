#include <stdio.h>

int readFile(char[], int[]);
void* arraySum(void*);

int readFile(char fileName[], int fileContents[]);{
    int successfulReads = 0;
    FILE fptr;

    fptr = fopen(fileName, "r");
    if(fptr == NULL){
        printf("File not found...");
        return -1;
    }
    
    while(fscanf(fptr, "%d", fileContents) == 1){
        successfulReads++;
    }
    fclose();
    return successfulReads;
    
}

void* arraySum(void* inputData){

}

int main(){

}