#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct state state;
typedef struct fun_desc fun_desc;
void toggleDebug(state *s);
void setFileName(state *s);
void setUnitSize(state *s);
void loadIntoMemory(state *s);
void toggleDisplay(state *s);
void memoryDisplay(state *s);
void saveIntoFile(state *s);
void memoryModify(state *s);
void quit(state *s);
//my
void print_debug_req(state *s);
char* curr_format(int flag, state *s);

//global
int dis_mode = 0;

typedef struct state {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
} state;

typedef struct fun_desc {
  char *name;
  void (*fun)(state*);
} fun_desc;

//main
int main(int argc, char **argv){  
    
    int option, func_amount = 0;

    state *state = calloc(1, sizeof(*state));
    state->debug_mode = 0;
    state->unit_size = 1;
    
    fun_desc menu[] = { {"Toggle Debug Mode",&toggleDebug},
                        {"Set File Name",&setFileName},
                        {"Set Unit Size",&setUnitSize},
                        {"Load into memory",&loadIntoMemory},
                        {"Toggle Display Mode",&toggleDisplay},
                        {"Memory Display", &memoryDisplay},
                        {"Save Into File",&saveIntoFile},
                        {"Memory Modify", &memoryModify},
                        {"Quit",&quit},{NULL,NULL}};

    for(int i = 0; menu[i].fun != NULL; i++){
        func_amount++;
    }
    while (1) {
        if(state->debug_mode)
            print_debug_req(state);

        printf("Choose action:\n");
        for(int i = 0; i < func_amount; i++){
            printf("%d-%s\n",i,menu[i].name);
        }
        scanf("%d",&option);
        fgetc(stdin); //avoiding '\n' in stdin
        if(option < 0 || option >= func_amount)
            fprintf(stderr, "invalid option bounds\n\n");
        else
            menu[option].fun(state);   
    }
    return 0;
}

void toggleDebug(state *s){
    if(!(s->debug_mode)){
        s->debug_mode = 1;
        fprintf(stderr, "Debug flag now on\n");
    }
    else {
        s->debug_mode = 0;
        fprintf(stderr, "Debug flag now off\n");
    }
}

void setFileName(state *s){
    printf("Enter file name: ");
    scanf("%s",s->file_name);
    
    if(s->debug_mode)
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
}

void setUnitSize(state *s){
    int tmp_size;
    printf("Enter unit size: ");
    scanf("%d",&tmp_size);

    if ( tmp_size!=1 && tmp_size!=2 && tmp_size!=4 )
        fprintf(stderr, "error - invalid unit size\n");
    else
        s->unit_size = tmp_size;

    if(s->debug_mode)
        fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
}

void loadIntoMemory(state *s){
    long file_size = 0;
    char in[10000];
    int location, length;

    if( *(s->file_name) == 0){
        fprintf(stderr, "file_name is NULL\n");
        return;
    }
    else {

        FILE * fd = fopen(s->file_name, "rb");
        if (fd == NULL) {
            perror("Open file");
            fprintf(stderr, "%p\n", s->file_name);
            return;
        }
        else {
            printf("Please enter <location> <length>\n");
            fgets(in, sizeof(in), stdin);

            sscanf(in, "%x %d", &location, &length);

            fseek(fd, 0, SEEK_END);
            file_size = ftell(fd);
            fseek(fd, 0, SEEK_SET);

            if(location > file_size){
                fprintf(stderr, "Target location is greater then the size of file");
                return;
            }
            if(fseek(fd, location, SEEK_SET) == -1){
                perror("fseek");
                return;
            }
            
            fread(s->mem_buf, s->unit_size, length, fd);
            s->mem_count+=s->unit_size*length;//

            if(s->debug_mode)
                fprintf(stderr, "Debug: copy to memory %d units(%d) from file '%s' at location: %x\n", length, s->unit_size, s->file_name, location);
            
            fclose(fd);
        }
    }
}

void toggleDisplay(state *s){
    if(!(dis_mode)){
        dis_mode = 1;
        fprintf(stderr, "Display flag now on, hexadecimal representation\n");
    }
    else {
        dis_mode = 0;
        fprintf(stderr, "Display flag now off, decimal representation\n");
    }  
}

void memoryDisplay(state *s){

    int units_amount, addr;
    char in[10000], *start, *end;

    printf("Please enter <amount of units> <address>\n");
    
    fgets(in, sizeof(in), stdin);
    sscanf(in, "%d %x", &units_amount, &addr);
    
    if(addr == 0)
        start = s->mem_buf;
    else
        start = (char*)addr;

    end = start + (s->unit_size)*units_amount;

    if(s->debug_mode)
        fprintf(stderr, "Debug: display from memory %d units (%d) at address: %d\n", units_amount, s->unit_size, addr);
    
    if(dis_mode) // 1 = hexa
        printf("Hexadecimal\n=======\n");
    else  // 0 = decimal
        printf("Decimal\n=======\n");
    
    while (start < end) {  //print to stdout the data of the memory in the requested address
        int var = *((int*)(start));
        printf(curr_format(dis_mode, s), var);
        start += s->unit_size;
    }          
}

void saveIntoFile(state *s){
    long dist, file_size = 0;
    char in[10000];
    int source_address, target_location, length;

    FILE * fd = fopen(s->file_name, "r+");
    if(fd == NULL){
        perror("open file");
        return;
    }

    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(in, sizeof(in), stdin);
    sscanf(in, "%x %x %d", &source_address, &target_location, &length);

    // ensure that the target-location is inside the bounds of the file
    fseek(fd, 0, SEEK_END);
    file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    if (target_location > file_size){
        fprintf(stderr, "Target location is greater then the size of file");
        return;
    }
    //offset of the file now at target_location
    fseek(fd, target_location, SEEK_SET); 

    if(source_address == 0)
        fwrite(s->mem_buf , s->unit_size, length, fd);
    else 
        fwrite((char*)source_address, s->unit_size, length, fd);

    if(s->debug_mode)
        fprintf(stderr, "Debug: save to memory %d units (%d) to file '%s' at location: %x\n", length, s->unit_size, s->file_name, target_location);
    
    fclose(fd);
}

void memoryModify(state *s){
    int location, val;
    char in[10000];
    long dist;
    
    printf("Please enter <location> <val>\n");
    fgets(in, sizeof(in), stdin); 
    
    sscanf(in, "%x %x", &location, &val);

    if(s->debug_mode)
        fprintf(stderr, "Debug: location is: %x, val is: %x\n", location, val);

    memcpy(&(s->mem_buf[location]), &val, s->unit_size);
}

void quit(state *s){
    if(s->debug_mode == 1)
        fprintf(stderr, "quitting\n");
    
    free(s);
    exit(0);
}

void print_debug_req(state *s){
    fprintf(stderr, "\nDebug:\n");
    fprintf(stderr, "unit_size: %d\n", s->unit_size);
    if(*(s->file_name) == 0)
        fprintf(stderr, "file_name: none\n");
    else
        fprintf(stderr, "file_name: %s\n", s->file_name);
    fprintf(stderr, "mem_count: %zu\n\n", s->mem_count);
}

char* curr_format(int flag, state *s) {
    static char* formats_for_hex[] = {"%hhx\n", "%hx\n", "No such unit", "%x\n"};
    if(flag) // flag == 1 it's hexa mode
        return formats_for_hex[s->unit_size-1];
    return "%d\n";
}