all: exec

c-lib: extendedShell.c pipes.c
	gcc -g -m32 -c -Include -o LineParser.o LineParser.c
	gcc -g -m32 -c -Include -o extendedShell.o extendedShell.c
	gcc -g -m32 -c -Include -o pipes.o pipes.c

exec: c-lib
	gcc -g -m32 extendedShell.o LineParser.o -o extendedShell
	gcc -g -m32 pipes.o LineParser.o -o pipes
	rm extendedShell.o
	rm pipes.o

.PHONY: clean
clean:
	rm -rf ./*.o extendedShell
	rm -rf ./*.o pipes