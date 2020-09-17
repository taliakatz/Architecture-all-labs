#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void encode(char * encoder, FILE * in, FILE * out);
void debug(FILE * in, FILE * out);
void encodeDebug(char * encoder, FILE * in, FILE * out);
void replaceUppercase(FILE * in, FILE * out);

int main(int argc, char **argv){

    char * outputName;
    char * inputName;
    FILE * ret;

    size_t j = 0;
    size_t lenFile;

    FILE * input = stdin;
    FILE * output = stdout;
    char * encoder;
    
    int flagUppercase = 0;
    int flagOut = 0;
    int flagIn = 0;
    int flagEnc = 0;
    int flagD = 0;
    int i = 1;
    int k = 0;
    size_t length;
    size_t counter;

    if(argc == 1){
        flagUppercase = 1;
    }
   
    for(counter = 1; counter < argc; counter++){
        
        if(strncmp(argv[i], "-e", 2) == 0 || strncmp(argv[i], "+e", 2) == 0){
            flagEnc = 1;
            encoder = argv[i];
        }
        
        else if((strncmp(argv[i], "-o", 2)) == 0){
            
            flagOut = 1;
            length = strlen(argv[i]);
            outputName = strndup(argv[i]+2, length-2);
            
                     
            output = fopen(outputName, "w+");
            if (output == NULL) { 
                fprintf(stderr, "%s\n", "Error cant open a file");
            }
        }
            
        else if((strncmp(argv[i], "-i", 2)) == 0){
            flagIn = 1;
            length = strlen(argv[i]);
            inputName = strndup(argv[i]+2, length-2);
            
            input = fopen(inputName, "r");

            if(input == NULL){
                fprintf(stderr, "ERROR file: %s not found\n", inputName);
            }
        }
        else if(strcmp(argv[i], "-D") == 0){
            flagD = 1;
        }
        i++;
    }
    
    if(flagEnc && flagD){
        encodeDebug(encoder, input, output);
    }
    else if(flagEnc){
        encode(encoder, input, output);
    }
    else if(flagD){
        debug(input, output);
    }
    else if(flagUppercase | flagOut | flagIn){
        replaceUppercase(input, output);
    }
    return 0;
}// end of main

//functions:
void encode(char * encoder, FILE * in, FILE * out){
    int c = 0;
    int t = 0;
    size_t i = 2;
    size_t len;
    len = strlen(encoder);
    while((c = fgetc(in)) != EOF){
        while(c != EOF & c != '\n'){
            if(i < len){
                t = encoder[i] - 48;
                if(encoder[0] == '+'){
                    c = c + t;
                }
                else if(encoder[0] == '-'){
                    c = c - t;
                }
                c = c % 128;
                fprintf(out, "%c", c);
                if(i == len-1){
                    i = 2;
                }
                else{
                    i++;
                }
            }
            c = fgetc(in);
        }
        fprintf(out, "%c", '\n');
        i = 2;
    }
    if(out != stdout){
        fclose(out);
    }
    if(in != stdin){
        fclose(in);
    }
}

void debug(FILE * in, FILE * out){
    
    char c;
    int upper = 0;   
    int lower = 0;
    FILE * tmp;
    while((c = fgetc(in)) != EOF){       
        tmp = fopen("tmp", "w+");
        while(c != EOF & c != '\n'){
            if(c >= 'a' && c <= 'z'){
                upper = c-32;
                lower = c;
                c = c-32;
            }
            else{
                upper = c;
                lower = c;
            }
            fprintf(tmp, "%c", c);
            fprintf(stderr, "%d 	%d\n", lower, upper);
            c = fgetc(in);
        }
        rewind(tmp);
        while((c = fgetc(tmp)) != EOF){
            fputc(c, out);
        }
        fputc('\n', out);
        fclose(tmp);
        remove("tmp");
    }
    if(out != stdout){
        fclose(out);
    }
    if(in != stdin){
        fclose(in);
    }
}

void encodeDebug(char * encoder, FILE * in, FILE * out){
    
    char e;
    int t = 0;
    size_t i = 2;
    size_t len;
    char c;
    int upper = 0;   
    int lower = 0;

    len = strlen(encoder);

    FILE * tmp1;
    FILE * tmp2;

    
    while((e = fgetc(in)) != EOF){
        tmp1 = fopen("tmp1", "w+");
        if (tmp1 == NULL) { 
                fprintf(stderr, "%s\n", "Error cant open a file");
        }
        while(e != EOF & e != '\n'){
            if(i < len){
                t = encoder[i] - 48;
                if(encoder[0] == '+'){
                    e = e + t;
                }
                else if(encoder[0] == '-'){
                    e = e - t;
                }
                e = e % 128;
                fprintf(tmp1, "%c", e);
                if(i == len-1){
                    i = 2;
                }
                else{
                    i++;
                }
            }
            e = fgetc(in);
        }
        i = 2;
        rewind(tmp1);
        tmp2 = fopen("tmp2", "w+");
        if (tmp2 == NULL) { 
                fprintf(stderr, "%s\n", "Error cant open a file");
        }
        while((c = fgetc(tmp1)) != EOF){       
            while(c != EOF & c != '\n'){
                if(c >= 'a' && c <= 'z'){
                    upper = c-32;
                    lower = c;
                    c = c-32;
                }
                else{
                    upper = c;
                    lower = c;
                }
                fprintf(tmp2, "%c", c);
                fprintf(stderr, "%d 	%d\n", lower, upper);
                c = fgetc(tmp1);
            }
            rewind(tmp2);
            while((c = fgetc(tmp2)) != EOF){
                fputc(c, out);
            }
            fputc('\n', out);
            fclose(tmp1);
            fclose(tmp2);
            remove("tmp1");
            remove("tmp2");
        }
    }
}

void replaceUppercase(FILE * in, FILE * out){

    int c = 0;
    while((c = fgetc(in)) != EOF){
        
        while(c != EOF & c != '\n'){
            if(c >= 97 && c <= 122){
                c = c-32;
            }
            fprintf(out, "%c", c);
                c = fgetc(in);
        }
        fprintf(out, "%c", '\n');
    }
}