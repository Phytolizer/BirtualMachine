.PHONY: all run clean
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Werror=implicit-function-declaration -Werror=missing-prototypes -Wswitch-enum

all: bm
run: all
	./bm
bm: main.o
	$(CC) $(CFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -vf *.o bm

