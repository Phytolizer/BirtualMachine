.PHONY: all clean
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Werror=implicit-function-declaration -Werror=missing-prototypes -Wswitch-enum
LIBS :=

all: ebasm bmi
ebasm: ebasm.c bm.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
bmi: bmi.c bm.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -vf *.o bm

.PHONY: examples
examples: ./examples/fib.bm ./examples/123.bm

./examples/%.bm: ./examples/%.ebasm ebasm
	./ebasm $< $@

