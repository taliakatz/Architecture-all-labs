#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct fun_desc fun_desc;
void toggleDebug();
void examineELFfile();
void printEntries();
void printSectionNames();
void printSymbols();
void relocationTables();
void toggleDebug();
void print_debug_req();
char* getType(int type);
void quit();


typedef struct fun_desc {
  char *name;
  void (*fun)();
} fun_desc;

//global
int currentfd = -1;
int dis_mode = 0;
int debug_mode = 0;
void *map_start; 
struct stat fd_stat; 
Elf32_Ehdr *header; 


int main(int argc, char **argv){  
    

    int option, func_amount = 0;
    fun_desc menu[] = { {"Toggle Debug Mode",&toggleDebug},
                        {"Examine ELF File", &examineELFfile},
                        {"Print Section Names", &printSectionNames},
                        {"Print Symbols", &printSymbols},
                        {"Relocation Tables", &relocationTables},
                        {"Quit",&quit},{NULL,NULL}};

    for(int i = 0; menu[i].fun != NULL; i++){
        func_amount++;
    }

    while (1) {
        

        printf("\nChoose action:\n");
        for(int i = 0; i < func_amount; i++){
            printf("%d-%s\n",i,menu[i].name);
        }
        scanf("%d",&option);
        fgetc(stdin); //avoiding '\n' in stdin
        if(option < 0 || option >= func_amount)
            fprintf(stderr, "invalid option bounds\n\n");
        else
            menu[option].fun();   
    }
    return 0;
}

void examineELFfile(){
    
    char file_name[1000];
   

    if(currentfd != -1){
        munmap(map_start, fd_stat.st_size);
		close(currentfd);
    }

    printf("Enter file name: ");

    scanf("%s", file_name);

    if( *(file_name) == 0){
        fprintf(stderr, "file_name is NULL\n");
        return;
    }

    if( (currentfd = open(file_name, O_RDWR)) < 0 ) {
        perror("Open file");
        fprintf(stderr, "%p\n", file_name);
        return;
    }

    if( fstat(currentfd, &fd_stat) != 0 ) {
        perror("stat failed");
        exit(-1);
    }

    if ((map_start = mmap(0, fd_stat.st_size, 
            PROT_READ | PROT_WRITE , MAP_SHARED, currentfd, 0)) == MAP_FAILED){
        perror("mmap");
        close(currentfd);
        currentfd = -1;
        return;
    }

    header = (Elf32_Ehdr *)map_start;
   
    printEntries();  

}

void printEntries(){
    char magic[4];
    if(strncmp(header->e_ident, ELFMAG , 4)!=0){
        perror("first 4 are not maching to ELF magic bytes");
        munmap(map_start, fd_stat.st_size);
        close(currentfd);
        currentfd = -1;
        return;
    }
    
    printf("\nMagic:\t\t\t\t\t%s\n", strncpy(magic, header->e_ident, 4));
    printf("Entry point address:\t\t\t%x\n",header->e_entry);
    printf("Section header table file offset:\t%x\n",header->e_shoff);
    printf("Number of section header entries:\t%d\n",header->e_shnum);
    printf("Size of each section header entry:\t%d\n",header->e_shentsize);
    printf("Program header table file offset:\t%x\n",header->e_phoff);
    printf("Number of program header entries:\t%d\n",header->e_phnum);
    printf("Size of each program header entry:\t%d\n",header->e_phentsize);
}

char* getType(int type){
    
    switch (type){
        case SHT_NULL:
            return "NULL";
        case SHT_PROGBITS:
            return "PROGBITS";
        case SHT_SYMTAB:
            return "SYMTAB";
        case SHT_STRTAB:
            return "STRTAB";
        case SHT_RELA:
            return "RELA";
        case SHT_HASH:
            return "HASH";
        case SHT_DYNAMIC:
            return "DYNAMIC";
        case SHT_NOTE:
            return "NOTE";
        case SHT_NOBITS:
            return "NOBITS";  
        case SHT_REL:
            return "REL";
        case SHT_SHLIB:
            return "SHLIB";
        case SHT_DYNSYM:
            return "DYNSYM";
        case SHT_INIT_ARRAY:
            return "INIT_ARRAY";
        case SHT_FINI_ARRAY:
            return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY:
            return "PREINIT_ARRAY";
        case SHT_GROUP:
            return "GROUP";
        case SHT_SYMTAB_SHNDX:
            return "SYMTAB_SHNDX";
        case SHT_NUM:
            return "NUM"; 
        case SHT_LOOS:
            return "LOOS";
        case SHT_GNU_LIBLIST:
            return "GNU_LIBLIST";
        case SHT_CHECKSUM:
            return "CHECKSUM";
        case SHT_LOSUNW:
            return "LOSUNW";
        case SHT_SUNW_COMDAT:
            return "SUNW_COMDAT";
        case SHT_SUNW_syminfo:
            return "SUNW_syminfo"; 
        case SHT_GNU_verdef:
            return "GNU_verdef";
        case SHT_GNU_verneed:
            return "GNU_verneed";
        case SHT_GNU_versym:
            return "GNU_versym";
        case SHT_LOPROC:
            return "LOPROC"; 
        case SHT_HIPROC:
            return "HIPROC";  
        case SHT_LOUSER:
            return "LOUSER"; 
        case SHT_HIUSER:
            return "HIUSER";   
	}
	return "Illegal type";
}

void printSectionNames(){
    if (currentfd == (-1)){
		fprintf(stderr, "the current fd is invalid\n");
		return;
	} 
    //pointer to the first section header (section header table)
    Elf32_Shdr* section = (Elf32_Shdr*)(map_start + header->e_shoff); 
    if(section == NULL){
        fprintf(stderr, "section header table NULL\n");
        return;
    }
    //pointer to the string table
    char* section_strtab = (char*)map_start + section[header->e_shstrndx].sh_offset;
    
    if(debug_mode)
        printf("%s%d\n%-7s%-18s%-19s%s\t%s\t%s\t%s\n", 
            "Index of string table(shstrndx): ", header->e_shstrndx,"Index",
            "Name Offset","Name","Address", "Offset","Size","Type");
    
    else 
        printf("%-8s%-20s%s\t%s\t%s\t%s\n",
            "Index","Name","Address","Offset","Size","Type");
    
    for (int i = 0; i < header->e_shnum; i++){
		printf("[%d%-5s", i, "]");
        if(debug_mode)
            printf("%06x\t\t",section[i].sh_name);  // section name offset
        printf("%-20s", section_strtab + section[i].sh_name);
        printf("%08x\t", section[i].sh_addr);
        printf("%06x\t", section[i].sh_offset);
        printf("%06d\t", section[i].sh_size);
        printf("%-15s\n" , getType(section[i].sh_type));
        
	}
}

// void getName_strtab(Elf32_Shdr* section, int index){
    
//     char* section_strtab = (char*)map_start + section[header->e_shstrndx].sh_offset;
//     if (index != 0 && index != 65521){
// 		return (char*)(section_strtab + section[index].sh_name);;
// 	}
//     return "";
// }

void printSymbols(){

    Elf32_Sym *symtab_struct ;
    char *section_strtab;
	char *sym_strtab;
    int entries_number;

    if (currentfd == (-1)) {
		fprintf(stderr, "the current fd is invalid\n");
		return;
	}
    Elf32_Shdr *section = (Elf32_Shdr *)(map_start + header->e_shoff);
    if(section == NULL){
        fprintf(stderr, "section header table NULL\n");
        return;
    }

    section_strtab = (char*)(map_start+section[header->e_shstrndx].sh_offset);


    for (int i = 0; i < header->e_shnum; i++){
        
        if ( section[i].sh_type == SHT_SYMTAB || section[i].sh_type == SHT_DYNSYM ){
        
            symtab_struct = (Elf32_Sym*)(map_start + section[i].sh_offset);
            if(symtab_struct == NULL){
                fprintf(stderr, "symbol table NULL\n");
                return;
            }

            sym_strtab = (char*)(map_start+section[section[i].sh_link].sh_offset);
            
            entries_number = section[i].sh_size / section[i].sh_entsize;

            printf("\n%s\n", section_strtab + section[i].sh_name); // dynsym | symtab
            
            if (debug_mode)
                printf("%-20s%-7s%-10s%-19s%-29s%-18s\n", "Size of symbol: ","Index","Value","Section Index","Symbol Name","Section Name");
            else
                printf("%-7s%-10s%-19s%-29s%-18s\n", "Index","Value","Section Index","Symbol Name","Section Name");

            for ( int j = 0; j < entries_number; j++ ){
                
                if(debug_mode)
                    printf("%-20d", symtab_struct[j].st_size);

                printf("[%d%-5s", j, "]");

                printf("%-10x" ,symtab_struct[j].st_value);  // value

                if(symtab_struct[j].st_shndx == 65521)
                    printf("%s\t\t" , "ABS"); // section index, the symbol has an absolute value
                else if(symtab_struct[i].st_shndx == 0)
                    printf("%s\t\t" , "UND");   // undefined symbol
                else    
                    printf("%d\t\t" ,symtab_struct[j].st_shndx );  // section index

                
                printf("%-30s", (char*)(sym_strtab + symtab_struct[j].st_name));   // symbol name

                if(symtab_struct[j].st_shndx == 0 || symtab_struct[j].st_shndx == 65521 )   // check if there is no name
                    printf("%s\n" , ""); 
                
                else    
                    printf("%s\n" , section_strtab +  section[symtab_struct[j].st_shndx].sh_name );  // section name
            }
            
        }
        
    }
}

void relocationTables(){
    
    Elf32_Sym *dyntab;
    Elf32_Rel *reltab;
    char *dyntab_str;
    int entries_num;
   
    Elf32_Shdr *section = (Elf32_Shdr*)(map_start + header->e_shoff);
    if(section == NULL){
        fprintf(stderr, "section header table NULL\n");
        return;
    }
    char *section_strtab = (char*)(map_start + section[header->e_shstrndx].sh_offset);
    
    for(int i = 0; i < header->e_shnum; i++ ){                
        
         if(section[i].sh_type == SHT_DYNSYM){
            dyntab = (Elf32_Sym*)(map_start + section[i].sh_offset);
            dyntab_str = (char*)(map_start+section[section[i].sh_link].sh_offset); 
        }

    }
    for (int i = 0; i < header->e_shnum; i++){
        
        if ( section[i].sh_type == SHT_REL ){
        
            reltab = (Elf32_Rel*)(map_start + section[i].sh_offset);
            if(reltab == NULL){
                fprintf(stderr, "relocation table NULL\n");
                return;
            }
        
           entries_num = section[i].sh_size / section[i].sh_entsize;
        
            printf("Relocation section '%s' at offset %#x contains %d entries:\n",(char*)(section_strtab + section[i].sh_name),section[i].sh_offset,entries_num);
            printf("Offset\t\tInfo\t\tType\tSym.Value\tSym. Name \n");
        
            for (int j = 0; j < entries_num; j++){

                printf("%08x\t%08x\t", (int)(reltab[j].r_offset), (int)(reltab[j].r_info)); // offset and info
                printf("%d\t", (int)(ELF32_R_TYPE(reltab[j].r_info)));   // type
                printf("%08x\t", (int)(dyntab[ELF32_R_SYM(reltab[j].r_info)].st_value));   // use index in smbol table to get the value
                printf("%s\n", (char*)(dyntab_str + dyntab[ELF32_R_SYM(reltab[j].r_info)].st_name));  // use index in smbol table to get the name
            }   
        }
    }
}

void toggleDebug(){
    debug_mode = !debug_mode;
    
    if(debug_mode){
        debug_mode = 1;
        fprintf(stderr, "Debug flag now on\n");
    }
    else 
        fprintf(stderr, "Debug flag now off\n");
}

void quit(){   
    if(debug_mode)
        fprintf(stderr, "quitting\n");
    munmap(map_start, fd_stat.st_size);
	close(currentfd);
    
    exit(0);
}