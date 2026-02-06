#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2);
int parseInput(char* input, char splitWords[][500], int maxWords);
int findElement(char splitWords[][500], int maxTokens, char elementToFind);
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
                int status1, status2;
                close(pipeFD2[0]);
                close(pipeFD2[1]);
                waitpid(pid, &status1, 0);
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


int findElement(char splitWords[][500], int maxTokens, char elementToFind){
    for(int i = 0; i < maxTokens; i++){
        if(splitWords[i] == elementToFind){
            return i;
        }
    }
}


void changeDirectories(const char* path){
    if(path == nullptr){
        perror("Path Not Formatted Correctly!");
    }
    if(chdir(path) == -1){
        perror("chdir Failed");
    }
}


int main(void){
    int maxWords = 100;
    char netID = "ahsu2";
    char userCommand[500];
    char splitWords[maxWords][500];
    char currentWorkingDirectory[500];
    getcwd(currentWorkingDirectory, 500);

    do{
        printf("<%s>:<%s>", netID, currentWorkingDirectory);
        fgets(userCommand, 500, stdin);
        size_t len = strlen(userCommand);
        if(len >0 && userCommand[len - 1] == "\n"){
            userCommand[len - 1] == "\0";
        }
        int numTokens = parseInput(userCommand, splitWords, maxWords);

        if(strcmp("exit", splitWords[0])){
            return(0);
        }
        else if(strcmp("cd", splitWords[0])){
            changeDirectories(userCommand);
        }
        else{
            char** fullCommand[numTokens + 1];
            char** fullCommand2[numTokens + 1];
            if(!strchr(userCommand, ">") && !strchr(userCommand, "<") && !strchr(userCommand, "|")){
                fullCommand[0] = splitWords[0];
                for(int i = 1; i < numTokens; i++){
                    if(i == numTokens){
                        fullCommand[i] = nullptr;
                    }
                    fullCommand[i] = splitWords[i]
                }
                executeCommand(fullCommand, nullptr, nullptr, nullptr);
            }
            else{
                if(strchr(userCommand, ">")){
                    int redirectIdx = findElement(splitWords, ">");
                    const char* inFile = splitWords[redirectIdx + 1];
                    for(int i = 1; i < numTokens; i++){
                        if(i == redirectIdx){
                            continue;
                        }
                        fullCommand[i] = splitWords[i]
                    }
                    for(int i = redirectIdx; i < numTokens - 1; i++){
                        fullCommand[i] = fullCommand[i + 1];
                    }
                    executeCommand(fullCommand, inFile, nullptr, nullptr);
                }
                else if(strchr(userCommand, "<")){
                    int redirectIdx = findElement(splitWords, ">");
                    const char* inFile = splitWords[redirectIdx + 1];
                    for(int i = 1; i < numTokens; i++){
                        if(i == redirectIdx){
                            continue;
                        }
                        fullCommand[i] = splitWords[i]
                    }
                    for(int i = redirectIdx; i < numTokens - 1; i++){
                        fullCommand[i] = fullCommand[i + 1];
                    }
                    executeCommand(fullCommand, nullptr, outFile, nullptr);
                }
                else if(strchr(userCommand, "|")){
                    int redirectIdx = findElement(splitWords, "|");
                    const char* inFile = splitWords[redirectIdx + 1];
                    for(int i = 1; i < redirectIdx; i++){
                        if(i == redirectIdx){
                            continue;
                        }
                        fullCommand1[i] = splitWords[i]
                    }
                    for(int i = redirectIdx + 1; i < numTokens - 1; i++){
                        if(i == redirectIdx){
                            continue;
                        }
                        fullCommand2[i] = splitWords[i]
                    }
                    executeCommand(fullCommand, nullptr, nullptr, fullCommand2);
                }
            }
        }
    }while(true);
}