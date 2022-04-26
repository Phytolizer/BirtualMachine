CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Werror=implicit-function-declaration -Werror=missing-prototypes -Wswitch-enum
LIBS :=
OBJS := src/basm.o src/bme.o src/debasm.o
DEPS := $(OBJS:.o=.d)

CPPFLAGS += -MMD -MP

.PHONY: all
all: basm bme debasm
basm: src/basm.o
	$(CC) $(CFLAGS) -o $@ $^
bme: src/bme.o
	$(CC) $(CFLAGS) -o $@ $^
debasm: src/debasm.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -vf $(OBJS) $(DEPS) basm bme debasm examples/*.bm

.PHONY: examples
examples: ./examples/fib.bm ./examples/123.bm

./examples/%.bm: ./examples/%.basm basm
	./basm $< $@

-include $(DEPS)

