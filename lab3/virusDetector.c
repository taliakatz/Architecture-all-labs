#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_LEN 128    /* maximal input size */

//declares
typedef struct link link;
typedef struct virus virus;
typedef struct funcOption funcOption;
link* list_append(link* virus_list, virus* data);
void list_print(link *virus_list, FILE *out);
void list_free(link *virus_list);
void printHex(unsigned char* buffer, size_t length, FILE *out);
virus* readVirus(FILE* input);
void printVirus(virus* virus, FILE* output);
char* getSignaturesFile();
link* createVirusList(link *virus_list, FILE *input);
link* loadSignatures(link* virus_list, char *file);
link* printSignatures(link* virus_list, char *file);
link* detectVirus(link* virus_list, char *file);
void detect_virus(char *buffer, unsigned int size, link *virus_list);
void printDetected(int i, virus *vir);
link *fixFile(link *virus_list, char *file);
void kill_virus(char *fileName, int signatureOffset, int signatureSize);
link* quit(link* virus_list, char *file);

//struct virus
typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

//struct link
struct link {
    link *nextVirus;
    virus *vir;
};

//struct funcOption
typedef struct funcOption {
  char *name;
  link* (*func)(link* virus_list, char *file);
} funcOption;

//functions:
//adding one virus link to the viruses LL
link* list_append(link* virus_list, virus* data){
    link *new_link = (link*) calloc(1, sizeof(link));
    new_link->vir = data;

    if(virus_list == NULL){
        new_link->nextVirus = NULL;
    }
    else{
        new_link->nextVirus = virus_list;
    }
    return new_link; 
}
 
//recursively print a whole LL, using 'printVirus' to print every virus struct
void list_print(link *virus_list, FILE *out){
    if(virus_list->nextVirus != NULL){
        list_print(virus_list->nextVirus, out);
    }
    printVirus(virus_list->vir, out);
}

//recursively free LL, before continue to 'next', free the data 
void list_free(link *virus_list){
    if(virus_list != NULL && virus_list->vir != NULL){
        free(virus_list->vir->sig);
        free(virus_list->vir);
    }
    if(virus_list->nextVirus != NULL){
        list_free(virus_list->nextVirus);
        free(virus_list->nextVirus);
    }
}

//print hexa representation of a string, limited by length
void printHex(unsigned char* buffer, size_t length, FILE *out){
    size_t i = 0;
    char c = 0;
   
    while(i<length){
        fprintf(out, "%02X ", buffer[i]);
        i++;
    }
    printf("\n");  
}

//auxiliary functions:
//return a virus from file
virus* readVirus(FILE* input){
    unsigned short *sigSize;
    virus *vir = (virus*) calloc(1, sizeof(virus));
    unsigned char *buffer = calloc(18, sizeof(char));
    unsigned char *sizes = calloc(2, sizeof(char));

    if(input == NULL){
        exit(1);
    }
    fread(buffer, sizeof(char), 18, input);
    strncpy(sizes, buffer, 2);
    sigSize = (unsigned short*) sizes;
    vir->SigSize = *sigSize;
    strncpy(vir->virusName, buffer+2, 16);
    vir->sig = calloc(*sigSize, sizeof(char));
    fread(vir->sig, sizeof(char), *sigSize, input);
    free(sizes);
    free(buffer);
    return vir;
}

//print a representation of one virus
void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "Signature:\n");
    printHex(virus->sig, virus->SigSize, output);
}

//for the 'loadSignatures'
char* getSignaturesFile(){
    char* buffer = calloc(MAX_LEN, sizeof(char));
    char* inputName = calloc(MAX_LEN, sizeof(char));
    printf("Please enter signature file: ");
    fgets(buffer, MAX_LEN, stdin);
    sscanf(buffer, "%s", inputName);
    free(buffer);
    return inputName;
}

//for the 'loadSignatures'
link* createVirusList(link *virus_list, FILE *input){
    int length;
    virus *vir;
    fseek(input, 0, SEEK_END);
    length = ftell(input);
    fseek(input, 0, SEEK_SET);
    while(length > 0){
        vir = readVirus(input);
        virus_list = list_append(virus_list, vir);
        length = length - 18 - vir->SigSize;
    }
    return virus_list;
    
}

//get an empty virus list and return LL of viruses, each virus as a different link
link* loadSignatures(link* virus_list, char *file){
    FILE *input;
    char* fileName = getSignaturesFile();
    input = fopen(fileName, "r");
    if(input == NULL){
        fprintf(stderr, "ERROR file: %s not found\n", fileName);
    }
    else{
        virus_list = createVirusList(virus_list, input);
        fclose(input);
    }
    free(fileName);
    return virus_list;
}

//get LL of viruses and print the list, using 'print_list'
link* printSignatures(link* virus_list, char *file){
    if(virus_list != NULL){
        list_print(virus_list, stdout);
    }
    return virus_list;
}

void printDetected(int i, virus *vir){
    printf("detection location in detected file: %d\n", i);
    printf("virus name: %s\n", vir->virusName);
    printf("size of the virus signature: %u\n\n", vir->SigSize);
}

//find viruses in a file
void detect_virus(char *buffer, unsigned int size, link *virus_list){
    bool printed = false;
    int i = 0;
    if(virus_list->nextVirus != NULL){
        detect_virus(buffer, size, virus_list->nextVirus);
    }
    printf("%s\n", virus_list->vir->virusName); 
    printf("sig size is %u\n\n", virus_list->vir->SigSize); 
    while(i <= size && !printed){
        if(memcmp(buffer+i, (void*)virus_list->vir->sig, virus_list->vir->SigSize) == 0){
            printDetected(i, virus_list->vir);
            printed = true;
        }
        i++; 
    }
}

//open a suspected file
link *detectVirus(link* virus_list, char *file){
    unsigned int size = 10000;
    int length;
    char *buffer = calloc(10000, sizeof(char));
    FILE *fileToDetect = fopen(file, "r+");
    if(fileToDetect != NULL && virus_list != NULL){
        
        fseek(fileToDetect, 0, SEEK_END);
        length = ftell(fileToDetect);
        fseek(fileToDetect, 0, SEEK_SET);
        if(length < size){
            size = length;
        }
        fread(buffer, sizeof(char), length, fileToDetect);
        fclose(fileToDetect);
        detect_virus(buffer, size, virus_list);
    }
    free(buffer);
    return virus_list;
}

//switch the virus signature to nop instruction (0x90)
void kill_virus(char *fileName, int signatureOffset, int signatureSize){
    FILE *toChange;
    char nop[signatureSize];
    for(int i = 0; i < signatureSize; i++){
        nop[i] = 0x90;
    }
    toChange = fopen(fileName, "w");
    if(toChange == NULL){
        fprintf(stderr, "ERROR file: %s not found\n", fileName);
    }
    else{
        fseek(toChange, signatureOffset, SEEK_SET);
        fwrite(nop, 1, signatureSize, toChange);
    }
    fclose(toChange);
}

link *fixFile(link *virus_list, char *file){
    int signatureOffset;
    int signatureSize;
    char *buffer = calloc(MAX_LEN, sizeof(char));
    printf("Please enter the location to fix from: ");
    fgets(buffer, MAX_LEN, stdin);
    sscanf(buffer, "%d", &signatureOffset);
    printf("Please enter the size of the virus signature: ");
    fgets(buffer, MAX_LEN, stdin);
    sscanf(buffer, "%d", &signatureSize);
    kill_virus(file, signatureOffset, signatureSize);
    free(buffer);
    return virus_list;
}

//before quiting free the memory of the list
link* quit(link* virus_list, char *file){
    if(list_free != NULL){
        list_free(virus_list);
    }
    exit(0);
}

//main:
int main(int argc, char **argv) {
    char* fileToDetect;
    int op = -1;
    link* virus_list = NULL;
    funcOption menu[] = {{"Load signatures", &loadSignatures}, {"Print signatures", &printSignatures}, {"Detect viruses", &detectVirus}, {"Fix flie", &fixFile}, {"Ouit", &quit}};
    if(argc > 1){
        for (int i = 1; i < argc; i++){
            if(strcmp(argv[i], "")!=0){
                fileToDetect = argv[i];
            }
        }  
    }
    //print a menu of function for the user's choise
    while(1){
        printf("Please choose a function:\n");
        for(int j = 0; j < 5; j++){
            printf("%d) %s\n", j+1, menu[j].name);
        }
        printf("Option: ");
        scanf("%d", &op);
        fgetc(stdin);
        if(op < 1 || op > 5){
            quit(virus_list, "");
        }
        virus_list = menu[op-1].func(virus_list, fileToDetect);
    }
    list_free(virus_list);
    return 0;
}
