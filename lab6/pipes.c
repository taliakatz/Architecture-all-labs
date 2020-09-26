#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include "LineParser.h"

int main(int argc, char **argv){
    pid_t pid1, pid2;
    int flagD = 0;
    int p[2];
    char * arguments_child1[3];
    char * arguments_child2[4];

    arguments_child1[0] = "ls";
    arguments_child1[1] = "-l";
    arguments_child1[2] = 0;

    arguments_child2[0] = "tail";
    arguments_child2[1] = "-n";
    arguments_child2[2] = "2";
    arguments_child2[3] = 0;

    if(argc > 1 && ( strncmp(argv[1],"-D",2) == 0 || strncmp(argv[1],"-d",2) == 0))
        flagD = 1;

    if (pipe(p) < 0){
        perror("pipe");
        exit(1);
    }

    if(flagD) fprintf(stderr,"parent_process>forking child1\n");
    //child1
    if ( !(pid1 = fork()) ) {
        if(flagD) fprintf(stderr,"child1>redirecting stdout to the write end of the pipe p\n"); 
        
        close(1); //stdout

        if( dup(p[1]) == -1){
            perror("dup");
            exit(1);
        }

        close(p[1]);
        
        if(flagD) fprintf(stderr,"child1>going to execute cmd: %s\n", arguments_child1[0]);
        
        execvp(arguments_child1[0], arguments_child1);
        _exit(1);
    }

    else {  //parent
         if(flagD) fprintf(stderr, "parent_process>closing the write end of the pipe p\n");
        
         close(p[1]);
        
         if(flagD){
            fprintf(stderr, "parent_process>created process with id: %d\n", pid1);
            fprintf(stderr,"parent_process>forking child1\n");
         } 
        
        //child2
         if ( !(pid2 = fork()) ) {
        
             if(flagD) fprintf(stderr,"child2>redirecting stdin to the read end of the pipe p\n");
        
             close(0); //stdin
            
             if( dup(p[0]) == -1 ){
                perror("dup");
                exit(1);
             }
             close(p[0]);
                        
             if(flagD) fprintf(stderr,"child2>going to execute cmd: %s\n", arguments_child2[0]);

             execvp(arguments_child2[0], arguments_child2);
             _exit(1);
         }
         else {  //parent
            
            if(flagD){
                fprintf(stderr, "parent_process>created process with id: %d\n", pid2);
                fprintf(stderr,"parent_process>closing the read end of the pipe\n");
            }  
        
            close(p[0]);
        
            if(flagD) fprintf(stderr,"parent_process>waiting for child processes to terminate...\n");
        
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        
            if(flagD) fprintf(stderr,"parent_process>exiting...\n");
        }
    }

    return 0;
}
