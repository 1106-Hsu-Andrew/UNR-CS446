#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2);
void separateCommand(char* const* command, char* commandToken, char* argumentTokens[10]);
int parseInput(char* input, char splitWords[][500], int maxWords);
void changeDirectories(const char* path);
int main(void);

int executeCommand(char* const* command, const char* inFile, const char* outFile, char* const* command2){
    pid_t pid = fork();
    int pipeFD[2];
        if(command2 != NULL){
            if(pipe(pipeFD) < 0){
                perror("Pipe Error");
                exit(1);
            }

    if(pid < 0){
        perror("Fork Failed");
    }
    else if(pid == 0){
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

        char* commandToken;
        char* argumentTokens[10];
        separateCommand(command, commandToken, argumentTokens);
        int execResult = execvp(commandToken, argumentTokens);
        if(execResult == -1){
            perror("Exec Failed");
            _exit(execResult);
        }

    }
    else{
        int status;
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

                char* commandToken;
                char* argumentTokens[10];
                separateCommand(command2, commandToken, argumentTokens);
                int commandReturnVal = execvp(commandToken, argumentTokens);
                if(commandReturnVal == -1){
                    perror("Exec Failed");
                    _exit(commandReturnVal);
                }
            }
            else{
                close(pipeFD2[0]);
                close(pipeFD2[1]);
                waitpid(pid2, &status, 0);

                printf("Child finished with error status: %d", status);
                if(WIFEXITED(status)){
                    return 0;
                }
                else{
                    return -1;
                }

            }

        }
        else{
            wait(&status);
            printf("Child finished with error status: %d", status);
                if(WIFEXITED(status)){
                    return 0;
                }
                else{
                    return -1;
                }
        }
    }
}

void separateCommand(char* const* command, char* commandToken, char* argumentTokens[10]){
    int maxWords = 100;
    char splitWords[maxWords][500];
    int numTokens = parseInput(command, splitWords, maxWords);
    char* command = splitWords[0];
    for(int i = 0; i < numTokens - 1; i++){
            argumentTokens[i] = splitWords[i + 1];
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


int main(void){
    int maxWords = 100;
    char splitWords[maxWords][500];



}