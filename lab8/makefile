all: exec

c-lib: elf.c
	gcc -g -m32 -c -o elf.o elf.c

exec: c-lib
	gcc -g -m32 elf.o -o elf
	
.PHONY: clean
clean:
	rm -rf ./*.o elf