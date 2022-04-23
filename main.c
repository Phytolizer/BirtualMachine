#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BM_STACK_CAPACITY 1024

#define TRAPS_X \
	X(ok) \
	X(stack_overflow) \
	X(stack_underflow) \
	X(illegal_inst)

typedef enum {
#define X(name) trap_##name,
	TRAPS_X
#undef X
} Trap;

static const char* trap_as_cstr(Trap trap) {
	switch (trap) {
#define X(name) \
	case trap_##name: \
		return "trap_" #name;
		TRAPS_X
#undef X
		default:
			assert(false && "unreachable");
	}
}

typedef int64_t Word;
#define PRI_WORD PRId64

typedef struct {
	Word stack[BM_STACK_CAPACITY];
	size_t stack_size;
} Bm;

#define INST_TYPES_X \
	X(push) \
	X(plus)

typedef enum {
#define X(name) inst_type_##name,
	INST_TYPES_X
#undef X
} InstType;

typedef struct {
	InstType type;
	Word operand;
} Inst;

#define INST_PUSH(value) \
	{ .type = inst_type_push, .operand = (value) }
#define INST_PLUS \
	{ .type = inst_type_plus }

Bm bm = {0};

static Trap bm_execute_inst(Bm* bm, Inst inst) {
	switch (inst.type) {
		case inst_type_push:
			if (bm->stack_size >= BM_STACK_CAPACITY) {
				return trap_stack_overflow;
			}
			bm->stack[bm->stack_size++] = inst.operand;
			break;
		case inst_type_plus:
			if (bm->stack_size < 2) {
				return trap_stack_underflow;
			}
			bm->stack[bm->stack_size - 2] += bm->stack[bm->stack_size - 1];
			bm->stack_size--;
			break;
		default:
			return trap_illegal_inst;
	}
	return trap_ok;
}

static void bm_dump(const Bm* bm, FILE* stream) {
	fprintf(stream, "Stack:\n");
	for (size_t i = 0; i < bm->stack_size; i++) {
		fprintf(stream, "    %" PRI_WORD "\n", bm->stack[i]);
	}
}

static const Inst program[] = {
		INST_PUSH(34),
		INST_PUSH(35),
		INST_PLUS,
};

#define ARRAY_LEN(a) (sizeof(a) / sizeof(*(a)))

int main(void) {
	for (size_t i = 0; i < ARRAY_LEN(program); i++) {
		Trap trap = bm_execute_inst(&bm, program[i]);
		if (trap != trap_ok) {
			fprintf(stderr, "Trap activated: %s\n", trap_as_cstr(trap));
			bm_dump(&bm, stderr);
			return 1;
		}
	}

	bm_dump(&bm, stdout);
}
