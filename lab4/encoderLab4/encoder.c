#include "util.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_UNLINK 10
#define SYS_LSEEK 19
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern int system_call(int, ...);
void replaceUppers(int flagD, int in, int out, int tmp_file);
void debug_file(int sysId, int ret, int tmp_file);
void print_debugFile(int tmp_file);
void printPath(char *in_name, char *out_name);
void exitErr(int code);

int main (int argc , char* argv[], char* envp[]){
    int i, flagD, in_fd, out_fd, tmp_file;
    char *out_name, *in_name;
    in_fd = STDIN;
    out_fd = STDOUT;
    out_name = "stdout";
    in_name = "stdin";
    tmp_file = system_call(SYS_OPEN,"tmp",64|2,0777);
    if(tmp_file < 0) system_call(SYS_EXIT, 85);
    debug_file(SYS_OPEN, tmp_file, tmp_file);

    for (i = 1; i < argc; i++){
        if(strcmp(argv[i], "-D") == 0)
            flagD = 1;
        if(strncmp(argv[i], "-o", 2) == 0){
            out_name = argv[i]+2;
            out_fd = system_call(SYS_OPEN,out_name,64|2,0777);
            debug_file(SYS_OPEN, out_fd, tmp_file);
        }
        if(strncmp(argv[i], "-i", 2) == 0){
            in_name = argv[i]+2;
            in_fd = system_call(SYS_OPEN,in_name,0,0777);
            debug_file(SYS_OPEN, in_fd, tmp_file);
        }
    }
    replaceUppers(flagD, in_fd, out_fd, tmp_file);
    in_fd = system_call(SYS_CLOSE, in_fd);
    debug_file(SYS_CLOSE, in_fd, tmp_file);
    if(in_fd < 0) exitErr(85);
    out_fd = system_call(SYS_CLOSE, out_fd);
    debug_file(SYS_CLOSE, out_fd, tmp_file);
    if(out_fd < 0) exitErr(85);
    if(flagD){
        printPath(in_name, out_name);
        print_debugFile(tmp_file);
    } 
    system_call(SYS_CLOSE, tmp_file);
    system_call(SYS_UNLINK, "tmp");
    return 0;
}

void replaceUppers(int flagD, int in, int out, int tmp_file){
    int read, write;
    char c[1];
    read = system_call(SYS_READ,in,c,1);
    debug_file(SYS_READ, read, tmp_file);
    while (read && c[0] != (-1)){ 
        while(read && c[0] != '\n'){
            if(strcmp("Z", c) < 0) c[0]-= 32; 
            write = system_call(SYS_WRITE,out,c,1);
            debug_file(SYS_WRITE, write, tmp_file);
            read = system_call(SYS_READ,in,c,1);
            debug_file(SYS_READ, read, tmp_file);
            
        }
        system_call(SYS_WRITE,out,"\n",1);
        read = system_call(SYS_READ,in,c,1);
        debug_file(SYS_READ, read, tmp_file);
    } 
}

void debug_file(int sysId, int ret, int tmp_file){
    system_call(SYS_WRITE,tmp_file,"system call ID: ",16);
    system_call(SYS_WRITE,tmp_file,itoa(sysId),strlen(itoa(sysId)));
    system_call(SYS_WRITE,tmp_file,"   ",3);
    system_call(SYS_WRITE,tmp_file,"return value: ",14);
    system_call(SYS_WRITE,tmp_file,itoa(ret),strlen(itoa(ret)));
    system_call(SYS_WRITE,tmp_file,"\n",1);
}

void print_debugFile(int tmp_file){
    int read;
    char c[1];
    system_call(SYS_LSEEK, tmp_file, 0, SEEK_SET);
    read = system_call(SYS_READ, tmp_file, c, 1);
    while(read){
        system_call(SYS_WRITE,STDERR,c,1);
        read = system_call(SYS_READ, tmp_file, c, 1);
    }
}

void printPath(char *in_name, char *out_name){  
    system_call(SYS_WRITE,STDERR,"Input path: ",12);
    system_call(SYS_WRITE,STDERR,in_name,strlen(in_name));
    system_call(SYS_WRITE,STDERR,"\n",1);
    system_call(SYS_WRITE,STDERR,"Output path: ",13);
    system_call(SYS_WRITE,STDERR,out_name,strlen(out_name));
    system_call(SYS_WRITE,STDERR,"\n",1);
}

void exitErr(int code){
    system_call(SYS_EXIT, code, 0, 0);
}
