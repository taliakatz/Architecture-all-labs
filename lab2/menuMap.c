#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//functions

char censor(char c) {
    if(c == '!')
        return '.';
    else
        return c;
}

char encrypt(char c){
    if(c >= 0x20 && c <= 0x7E){
        c = c + 3;
    }
    return c;
}

char decrypt(char c){
    if(c >= 0x20 && c <= 0x7E){
        c = c - 3;
    }
    return c;
}

char dprt(char c){
    printf("%d\n", c);
    return c;
}

char cprt(char c){
    if(c >= 0x20 && c <= 0x7E){
        printf("%c\n", c);
    }
    else{
        printf("%c\n", '.');
    }
    return c;
}

char my_get(char c){
    return fgetc(stdin);
}

char quit(char c){
    if(c == 'q' || c == 'Q'){
        exit(0);
    }
}

//map
char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    int i;
    for(i = 0; i<array_length; i++){
        *(mapped_array+i) = f(*(array+i));
    }
    return mapped_array;
}

//struct
struct fun_desc {
  char *name;
  char (*fun)(char);
};

void menu(){
    
    //declare of variables
    int op, i;
    int bound = 0;
    int base_len = 5;
    char *arr;
    char *carray = (char*)(malloc(base_len*sizeof(char)));
    carray[0] = '\0';
    
    //build an arrau
    struct fun_desc menu[] = { {"Censor", &censor}, {"Encrypt", &encrypt},{"Decrypt", &decrypt}, {"Print dec", &dprt}, {"Print string", &cprt}, {"Get string", &my_get}, {"Quit", &quit}, { NULL, NULL } };
    
    //calculate the length of fun_desc array
    for(i = 0; menu[i].fun != NULL; i++){
        bound++;
    }
    
    //loop over fun_desc
    op = -1;
    while(1){
        printf("Please choose a function:\n");
        for(i = 0; i < bound; i++){
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option: ");
        scanf("%d", &op);
        fgetc(stdin);
        printf("%d\n", op);
        if(op >= 0 && op <= bound){
            printf("Within bounds\n");
        }
        else{
            printf("Not within bounds");
            quit('q');
        }
        arr = map(carray, base_len, menu[op].fun);
        carray = arr;
        printf("DONE\n\n");
    }
}

//main
int main(int argc, char **argv){  
    menu();
    return 0;
}
