.PHONY: all run clean
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Werror=implicit-function-declaration -Werror=missing-prototypes -Wswitch-enum
LIBS :=

all: ebasm bmi
run: all
	./bm
ebasm: ebasm.c bm.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
bmi: bmi.c bm.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -vf *.o bm

