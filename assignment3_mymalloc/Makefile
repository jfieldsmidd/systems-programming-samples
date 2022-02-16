.PHONY: all
all: my-malloc.so test-my-malloc

my-malloc.so: my-malloc.c
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c

test-my-malloc: test-my-malloc.c
	gcc -Wall -pedantic -o test-my-malloc test-my-malloc.c

test-my-malloc2: test-my-malloc2.c
	gcc -Wall -pedantic -o test-my-malloc2 test-my-malloc2.c

.PHONY: test-malloc
test-malloc: my-malloc.so test-my-malloc
	LD_PRELOAD=./my-malloc.so ./test-my-malloc

.PHONY: test-malloc-gdb
test-malloc-gdb:  my-malloc.so test-my-malloc
	gdb --args env LD_PRELOAD=./my-malloc.so ./test-my-malloc

.PHONY: test-malloc2
test-malloc2: my-malloc.so test-my-malloc2
	LD_PRELOAD=./my-malloc.so ./test-my-malloc2

.PHONY: test-malloc2-gdb
test-malloc2-gdb:  my-malloc.so test-my-malloc2
	gdb --args env LD_PRELOAD=./my-malloc.so ./test-my-malloc2

.PHONY: test-ls
test-ls: my-malloc.so
	LD_PRELOAD=./my-malloc.so ls

.PHONY: test-ls-gdb
test-ls-gdb:  my-malloc.so
	gdb --args env LD_PRELOAD=./my-malloc.so ls

.PHONY: test-lsl
test-lsl: my-malloc.so
	LD_PRELOAD=./my-malloc.so ls -l

.PHONY: test-lsl-gdb
test-lsl-gdb:  my-malloc.so
	gdb --args env LD_PRELOAD=./my-malloc.so ls -l

.PHONY: test-ls-hard
test-ls-hard: my-malloc.so
	LD_PRELOAD=./my-malloc.so ls -lR /usr

.PHONY: test-ls-hard-gdb
test-ls-hard-gdb:  my-malloc.so
	gdb --args env LD_PRELOAD=./my-malloc.so ls -lR /usr

.PHONY: clean
clean:
	rm -f my-malloc.so
	rm -f test-my-malloc
	rm -f test-my-malloc2
