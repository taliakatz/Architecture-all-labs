#include <linux/limits.h>
#include "LineParser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

//globals
int flagD = 0;
//declares
typedef struct pair pair;
void execute(cmdLine *pCmdLine);
pair* addPair(pair *pair_list, char *var, char *value);
void printVars(pair *pair_list);
void freePairList(pair *pair_list);
char* foundVar(char *find, pair *pair_list);

//type define
typedef struct pair {
    char *var;
    char *value;
    pair* next;
} pair;


int main(int argc, char **argv){
    int check;
    pid_t pid, pid2;
    int p[2];
    char cwd[PATH_MAX];
    char buf [2048];
    char *replace = NULL;
    pair *pair_list = NULL;
   
    if(argc >1 && strncmp(argv[1],"-D",2)==0)
        flagD = 1;

    while(1) {
        
        getcwd(cwd, PATH_MAX);
        printf("Current working dir: %s\n", cwd); //present the current directory

        fgets(buf, 2048, stdin);

        if(feof(stdin))
            break;

        cmdLine *pCmdLine = parseCmdLines(buf);
        
        for (int i = 0; i < pCmdLine->argCount; i++) {
            if( strncmp(pCmdLine->arguments[i], "$", 1) == 0 ){
                replace = foundVar(pCmdLine->arguments[i], pair_list);
                if (replace != NULL){
                    check = replaceCmdArg(pCmdLine, i,replace);
                    if(check != 1)
                        fprintf(stderr, "Error in replacing the value of variable");   
                }
                else
                    fprintf(stderr, "Activating a variable that does not exist\n");
            }
        }   

        if(strcmp(pCmdLine->arguments[0], "quit") == 0){
            freeCmdLines(pCmdLine);
            exit(0);
        }
        else if(strcmp(pCmdLine->arguments[0], "cd") == 0){
            fprintf(stderr, "PID: %d    Executing command: %s\n", getpid(), pCmdLine->arguments[0]);
            if( strcmp(pCmdLine->arguments[1],"~") == 0 ){
                char *home = getenv("HOME");
                if(chdir(home) == -1)
                    fprintf(stderr, "error cd\n");
            }
            else if( chdir(pCmdLine->arguments[1]) == -1 )
                fprintf(stderr, "error cd\n");
            freeCmdLines(pCmdLine);
        }
        else if ( strcmp(pCmdLine->arguments[0],"set") == 0 ){
            pair_list = addPair(pair_list, pCmdLine->arguments[1], pCmdLine->arguments[2]);
            freeCmdLines;
        }
        else if ( strcmp(pCmdLine->arguments[0],"vars") == 0 ){
            printVars(pair_list);
            freeCmdLines;
        }
        else {
            if( pCmdLine->next != NULL){
                if (pipe(p) < 0){
                    perror("pipe");
                    exit(1);
                }
                if ( !(pid = fork()) ){
                    if(flagD) fprintf(stderr, "PID: %d    Executing command: %s\n", getpid(), pCmdLine->arguments[0]);
                    close(1); //stdout
                    if( dup(p[1]) == -1){
                        perror("dup");
                        exit(1);
                    }
                    close(p[1]);
                    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
                    _exit(1);   
                }
                //parent
                close(p[1]);
                
                if( !(pid2 = fork()) ) {
                    if(flagD) fprintf(stderr, "PID: %d    Executing command: %s\n", getpid(), pCmdLine->arguments[0]);
                    close(0); //stdin     
                    if( dup(p[0]) == -1){
                        perror("dup");
                        exit(1);
                    }
                    close(p[0]);
                    execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments);
                    _exit(1);         
                }
                //parent again
                close(p[0]);
                waitpid(pid, NULL, 0);
                waitpid(pid2, NULL, 0);
            }
            else {
                if ( !(pid = fork()) ){
                    if(flagD) fprintf(stderr, "PID: %d    Executing command: %s\n", getpid(), pCmdLine->arguments[0]);
                    execute(pCmdLine);
                }
                if( pCmdLine->blocking ){
                    waitpid(pid, NULL, 0);
                }
            }  
        }
    }
    return 0;
}

void execute(cmdLine *pCmdLine){
    if(pCmdLine->inputRedirect != NULL){
            close(0); //closing stdin for child
            if ( open(pCmdLine->inputRedirect, 0, 0777) < 0)
                perror("open");
    }
    if(pCmdLine->outputRedirect != NULL){
        close(1); //closing stdout for child
        if ( open(pCmdLine->outputRedirect, 64|2, 0777) < 0)
            perror("open");
    }
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    _exit(1);
}


pair* addPair(pair *pair_list, char *var, char *value){
    pair *newPair = (pair*) calloc(1, sizeof(pair));
    newPair->var = calloc(1, sizeof(var));
    newPair->value = calloc(1, sizeof(value));
    if ( var != NULL && var != "" && value != NULL && value != "" ){
        strcpy(newPair->var, var);
        strcpy(newPair->value, value);
        if ( pair_list == NULL )
            newPair->next = NULL;
        else
            newPair->next = pair_list;
    }

    return newPair;
}

void printVars(pair *pair_list){
    if ( pair_list->next != NULL )
        printVars(pair_list->next);
    printf("var: %s, value: %s\n", pair_list->var, pair_list->value);
}

void freePairList(pair *pair_list){
    if (pair_list->next->next != NULL){
        freePairList(pair_list->next);
        free(pair_list->next);
    }
}

char* foundVar(char *find, pair *pair_list){
    pair *tmp = pair_list;
    while (tmp != NULL){
        if( strcmp(pair_list->var, find+1 ) == 0 )
            return pair_list->value;
        tmp = tmp->next;
    }
    return NULL;
    
}