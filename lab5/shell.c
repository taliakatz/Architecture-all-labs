#include <linux/limits.h>
#include "LineParser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <signal.h>

#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

//declares
typedef struct process process;
void execute(cmdLine *pCmdLine);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void printProcessLink(process* process_link, int index);
void freeProcessList(process* process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);

//type define
typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;

//globals
int flagD = 0;

int main(int argc, char **argv){
    
    char cwd[PATH_MAX];
    char buf [2048];
    pid_t pid;

    process** process_list = malloc(sizeof(process));
    process_list[0] = NULL;
    
    if(argc > 1 && strncmp(argv[1],"-D",2) == 0)
        flagD = 1;

    while(1) {
        
        getcwd(cwd, PATH_MAX);
        printf("Current working dir: %s\n", cwd); // current directory

        fgets(buf, 2048, stdin);   //get a command line

        if(feof(stdin))
            break;

        cmdLine *pCmdLine = parseCmdLines(buf);

        if ( strcmp(pCmdLine->arguments[0], "quit" ) == 0){
            freeCmdLines(pCmdLine);
            freeProcessList(process_list[0]);
            free(process_list[0]);
            free(process_list);
            exit(0);
        }
        else if ( strcmp(pCmdLine->arguments[0],"cd") == 0 ){   
            if(chdir(pCmdLine->arguments[1]) == -1 )
                fprintf(stderr, "error cd\n");
            freeCmdLines(pCmdLine);
        }
        else if ( strcmp(pCmdLine->arguments[0],"procs") == 0 ){
            printProcessList(process_list);
            freeCmdLines(pCmdLine);
        }
        else if ( strcmp(pCmdLine->arguments[0],"suspend") == 0 ){
            if( kill(atoi(pCmdLine->arguments[1]), SIGTSTP) == 0)
                updateProcessList(process_list);
            freeCmdLines(pCmdLine);
        }
        else if ( strcmp(pCmdLine->arguments[0],"kill") == 0 ){
            if( kill(atoi(pCmdLine->arguments[1]), SIGINT) == 0)
                updateProcessList(process_list);
            freeCmdLines(pCmdLine);
        }
        else if ( strcmp(pCmdLine->arguments[0],"wake") == 0 ){
            if( kill(atoi(pCmdLine->arguments[1]), SIGCONT) == 0)
                updateProcessList(process_list);
            freeCmdLines(pCmdLine);
        }
        else {
            if ( !(pid = fork()) ){
                if(flagD){
                    fprintf(stderr, "PID: %d    Executing command: %s\n", getpid(), pCmdLine->arguments[0]);
                }
                execute(pCmdLine);
            }
            if( pCmdLine->blocking ){
                waitpid(pid, NULL, 0);
            }
            addProcess(process_list, pCmdLine, pid);
        } 
    }
    return 0;
}
void execute(cmdLine *pCmdLine){
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    _exit(1);
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    int i = 0;
    process *tmp;
    process *link = (process*)malloc(sizeof(process));
    link->cmd = cmd;
    link->pid = pid;
    link->status = RUNNING;
    link->next = NULL;
    if(process_list[0] == NULL){
        process_list[0] = link;
    }
    else{
        tmp = process_list[0];
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = link;
    }
}

void printProcessList(process** process_list){
    process *tmp, *prev, *after;
    int i = 0;
    if(process_list != NULL && process_list[0] != NULL){
        updateProcessList(process_list);
        
        tmp = process_list[0];
        prev = tmp;
        after = tmp->next;
        

        printf("index\tPID\tSTATUS\tCommand\n");
    
        while ( tmp != NULL ){
            printf("%d\t%d\t%d\t%s\n", i, tmp->pid, tmp->status, *(tmp->cmd->arguments));
            if(tmp->status != -1){
                prev = tmp;
                tmp = tmp->next;
                if(after != NULL)
                    after = after->next;
            }
            else {     // tmp->status == -1
                if(prev->pid != tmp->pid){
                    prev->next = after;
                    freeCmdLines(tmp->cmd);
                    free(tmp);
                    tmp = after;
                    if(after != NULL)
                        after = after->next;
                }
                else {
                    freeCmdLines(tmp->cmd);
                    free(tmp);
                    tmp = after;
                    prev = tmp;
                    if(after != NULL)
                        after = after->next;
                    process_list[0] = tmp;
                }
            }
            i++;
        }
    }
}

void freeProcessList(process* process_list){
    if(process_list!=NULL){
        if(process_list->cmd != NULL)
            freeCmdLines(process_list->cmd);
        
        if(process_list->next != NULL){
            freeProcessList(process_list->next);
            free(process_list->next);
        }
    }  
}

void updateProcessList(process **process_list){
    
    pid_t w;
    int status = 0, update;
    process *tmp = process_list[0];
    
    while (tmp != NULL) {
        
        update = tmp->status;
        
        w = waitpid(tmp->pid, &status, WNOHANG|WUNTRACED|WCONTINUED);
        
        if (w == -1) 
            update = TERMINATED;

        if(w != 0){
            
            if (WIFEXITED(status)) {
                update = TERMINATED;
            }

            else if(WIFSIGNALED(status)){
                update = TERMINATED;
            }

            else if(WIFSTOPPED(status))
                update = SUSPENDED;

            else if(WIFCONTINUED(status))
                update = RUNNING;
        }

        updateProcessStatus(tmp, tmp->pid, update);

        tmp = tmp->next;  
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    process_list->status = status;
}
