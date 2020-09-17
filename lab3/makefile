all: exec

c-lib: virusDetector.c
	gcc -g -m32 -c -o virusDetector.o virusDetector.c

exec: c-lib
	gcc -g -m32  virusDetector.o -o virusDetector
	rm virusDetector.o

.PHONY: clean
clean:
	rm -rf ./*.o virusDetector