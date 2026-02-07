#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>



int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2);
int parseInput(char* input, char splitWords[][500], int maxWords);
int findElement(char splitWords[][500], int maxTokens, const char* elementToFind);
void changeDirectories(const char* path);
int main(void);

int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2){
    int pipeFD[2];
    if(command2 != NULL){
        if(pipe(pipeFD) < 0){
            perror("Pipe Error");
            exit(1);
        }
    }

    pid_t pid = fork();
    if(pid < 0){
        perror("Fork Failed");
        return -1;
    }
    
    if(pid == 0){
        if(inFile != NULL && inFile != "\0"){
            int fdIn = open(inFile, O_RDONLY, 0666);
            dup2(fdIn, STDIN_FILENO);
            close(fdIn);
        }
        else if(outFile != NULL && outFile != "\0"){
            int fdOut = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
            dup2(fdOut, STDOUT_FILENO);
            close(fdOut);
        }
        else if(command2 != NULL){
            dup2(pipeFD[1], STDOUT_FILENO);
            close(pipeFD[0]);
            close(pipeFD[1]);
        }


        int execResult = execvp(command[0], command);
        if(execResult == -1){
            perror("Exec Failed");
            _exit(execResult);
        }

    }
    else{
        int pipeFD2[2];
        if(command2 != NULL){
            if(pipe(pipeFD2) < 0){
                perror("Pipe Error");
                exit(1);
            }

            pid_t pid2 = fork();
            if(pid2 < 0){
                perror("Fork Failed");
            }
            else if(pid2 == 0){
                dup2(pipeFD2[0], STDIN_FILENO);
                close(pipeFD2[0]);
                close(pipeFD2[1]);

                int commandReturnVal = execvp(command2[0], command2);
                if(commandReturnVal == -1){
                    perror("Exec Failed");
                    _exit(commandReturnVal);
                }
            }
            else{
                int status, status2;
                close(pipeFD2[0]);
                close(pipeFD2[1]);
                waitpid(pid, &status, 0);
                waitpid(pid2, &status2, 0);

                if(WIFEXITED(status) && WIFEXITED(status2) != 0){
                    printf("Child finished with error status: %d", status);
                    return -1;
                }
            }
        }
        else{
            int status;
            wait(&status);
            if(WIFEXITED(status) != 0){
                printf("Child finished with error status: %d", status);
                return -1;
            }
        }
    }
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


int findElement(char splitWords[][500], int maxTokens, const char* elementToFind){
    for(int i = 0; i < maxTokens; i++){
        if(strcmp(splitWords[i], elementToFind) == 0){
            return i;
        }
    }
    return -1;
}


void changeDirectories(const char* path){
    if(path == NULL){
        perror("Path Not Formatted Correctly!");
    }
    if(chdir(path) == -1){
        perror("chdir Failed");
    }
}


int main(void){
    int maxWords = 100;
    char* netID = "ahsu2";
    char userCommand[500];
    char splitWords[maxWords][500];
    char currentWorkingDirectory[500];
    getcwd(currentWorkingDirectory, 500);

    do{
        printf("<%s>:<%s> ", netID, currentWorkingDirectory);
        fgets(userCommand, 500, stdin);
        size_t len = strlen(userCommand);
        if(len >0 && userCommand[len - 1] == '\n'){
            userCommand[len - 1] = '\0';
        }
        int numTokens = parseInput(userCommand, splitWords, maxWords);
        if(strcmp("exit", splitWords[0]) == 0){
            return(0);
        }
        else if(strcmp("cd", splitWords[0]) == 0){
            changeDirectories(userCommand);
        }
        else{
            char** fullCommand = NULL;
            char** fullCommand2 = NULL;
            if(!strchr(userCommand, '>') && !strchr(userCommand, '<') && !strchr(userCommand, '|')){
                fullCommand = (char**)malloc((numTokens + 1) * sizeof(char*));
                for(int i = 0; i < numTokens; i++){
                    fullCommand[i] = splitWords[i];
                }
                fullCommand[numTokens] = NULL;
                executeCommand(fullCommand, NULL, NULL, NULL);
            }
            else{
                if(!strchr(userCommand, '>')){
                    int redirectIdx = findElement(splitWords, numTokens, ">");
                    const char* inFile = splitWords[redirectIdx + 1];
                    fullCommand = (char**)malloc((redirectIdx + 1) * sizeof(char*));
                    for(int i = 0; i < redirectIdx; i++){
                        fullCommand[i] = splitWords[i];
                    }
                    fullCommand[redirectIdx] = NULL;
                    executeCommand(fullCommand, inFile, NULL, NULL);
                }
                else if(!strchr(userCommand, '<')){
                    int redirectIdx = findElement(splitWords, numTokens, "<");
                    const char* outFile = splitWords[redirectIdx + 1];
                    fullCommand = (char**)malloc((redirectIdx + 1) * sizeof(char*));
                    for(int i = 0; i < redirectIdx; i++){
                        fullCommand[i] = splitWords[i];
                    }
                    fullCommand[redirectIdx] = NULL;
                    executeCommand(fullCommand, NULL, outFile, NULL);
                }
                else if(!strchr(userCommand, '|')){
                    int redirectIdx = findElement(splitWords, numTokens, "|");
                    fullCommand = (char**)malloc((redirectIdx + 1) * sizeof(char*));
                    for(int i = 0; i < redirectIdx; i++){
                        fullCommand[i] = splitWords[i];
                    }
                    fullCommand[redirectIdx] = NULL;

                    int k = 0;
                    fullCommand2 = (char**)malloc((numTokens - (redirectIdx + 1) + 1) * sizeof(char*));
                    for(int i = redirectIdx + 1; i < numTokens; i++){
                        fullCommand2[k++] = splitWords[i];
                    }
                    fullCommand2[redirectIdx] = NULL;
                    executeCommand(fullCommand, NULL, NULL, fullCommand2);
                }
                if(fullCommand!= NULL) free(fullCommand);
                if(fullCommand2!= NULL) free(fullCommand2);
            }
        }
    }while(1);
}