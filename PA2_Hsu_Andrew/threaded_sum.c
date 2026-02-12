#include <stdio.h>
#include <string.h>

int readFile(char[], int[]);
void* arraySum(void*);

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
    if(argc != 3){
        printf("There aren't enough parameters.");
        return -1;
    }

    int fileContents[1000000];

    int fileLen = readFile(argv[1], fileContents);
    for(int i = 0; i < fileLen; i++){
        printf("%d\n", fileContents[i]);
    }
}