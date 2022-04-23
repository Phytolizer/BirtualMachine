#include <stdint.h>
#include <stdio.h>

#define BM_STACK_CAPACITY 1024

typedef int64_t Word;

typedef struct {
	Word stack[BM_STACK_CAPACITY];
} Bm;

int main(void) {
	printf("hello world\n");
}
