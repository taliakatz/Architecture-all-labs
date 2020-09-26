all: exec 

asm-lib: infector.s
	nasm -f elf32 infector.s -o infector.o

exec: asm-lib
	ld -m elf_i386 infector.o -o infector
	
.PHONY: clean
clean:
	rm -rf ./*.o infector
