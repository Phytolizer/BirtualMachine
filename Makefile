.PHONY: all clean
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Werror=implicit-function-declaration -Werror=missing-prototypes -Wswitch-enum
LIBS :=

all: basm bme
basm: src/basm.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
bme: src/bme.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -vf *.o bm

.PHONY: examples
examples: ./examples/fib.bm ./examples/123.bm

./examples/%.bm: ./examples/%.basm basm
	./basm $< $@

