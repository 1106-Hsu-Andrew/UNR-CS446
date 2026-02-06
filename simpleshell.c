#include <string.h>
#include <unistd.h>
#include <stdio.h>


int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2);
int parseInput(char* input, char splitWords[][500], int maxWords);
void changeDirectories(const char* path);
int main(void);

int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2){

}

int parseInput(char* input, char splitWords[][500], int maxWords){
    int validTokenCount = 0;
    char* token = strtok(input, " ");

    while(token != NULL){
        strcpy(splitWords[validTokenCount], token);
        validTokenCount++;
        token = strtok(NULL, " ");
    }
    
    return validTokenCount;
}


int main(void){
    int maxWords = 100;
    char splitWords[maxWords][500];



}