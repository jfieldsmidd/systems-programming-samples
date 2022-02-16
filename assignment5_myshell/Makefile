CFLAGS=-g -Wall -pedantic

.PHONY: all
all: mysh

mysh: mysh.c
	gcc $(CFLAGS) -o $@ $^

mysh_old: mysh_old.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f mysh mysh_old strace.out output.txt
