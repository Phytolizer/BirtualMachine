#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 69
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define TRAPS_X \
	X(ok) \
	X(stack_overflow) \
	X(stack_underflow) \
	X(illegal_inst) \
	X(div_by_zero) \
	X(illegal_inst_access) \
	X(illegal_operand)

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

#define INST_TYPES_X \
	X(push) \
	X(plus) \
	X(minus) \
	X(mult) \
	X(div) \
	X(jump) \
	X(jump_if) \
	X(eq) \
	X(halt) \
	X(print_debug) \
	X(dup)

typedef enum {
#define X(name) inst_type_##name,
	INST_TYPES_X
#undef X
} InstType;

static const char* inst_type_as_cstr(InstType type) {
	switch (type) {
#define X(name) \
	case inst_type_##name: \
		return "inst_type_" #name;
		INST_TYPES_X
#undef X
		default:
			assert(false && "unreachable");
	}
}

typedef struct {
	InstType type;
	Word operand;
} Inst;

typedef struct {
	Word stack[BM_STACK_CAPACITY];
	size_t stack_size;

	Inst program[BM_PROGRAM_CAPACITY];
	size_t program_size;
	size_t ip;

	bool halt;
} Bm;

#define INST_PUSH(value) \
	{ .type = inst_type_push, .operand = (value) }
#define INST_PLUS() \
	{ .type = inst_type_plus }
#define INST_MINUS() \
	{ .type = inst_type_minus }
#define INST_MULT() \
	{ .type = inst_type_mult }
#define INST_DIV() \
	{ .type = inst_type_div }
#define INST_JUMP(dest) \
	{ .type = inst_type_jump, .operand = (dest) }
#define INST_HALT() \
	{ .type = inst_type_halt }
#define INST_JUMP_IF(dest) \
	{ .type = inst_type_jump_if, .operand = (dest) }
#define INST_EQ() \
	{ .type = inst_type_eq }
#define INST_DUP(ofs) \
	{ .type = inst_type_dup, .operand = (ofs) }

static Trap bm_execute_inst(Bm* bm) {
	if (bm->ip >= bm->program_size) {
		return trap_illegal_inst_access;
	}
	Inst inst = bm->program[bm->ip];
	switch (inst.type) {
		case inst_type_push:
			if (bm->stack_size >= BM_STACK_CAPACITY) {
				return trap_stack_overflow;
			}
			bm->stack[bm->stack_size++] = inst.operand;
			bm->ip++;
			break;
		case inst_type_plus:
			if (bm->stack_size < 2) {
				return trap_stack_underflow;
			}
			bm->stack[bm->stack_size - 2] += bm->stack[bm->stack_size - 1];
			bm->stack_size--;
			bm->ip++;
			break;
		case inst_type_minus:
			if (bm->stack_size < 2) {
				return trap_stack_underflow;
			}
			bm->stack[bm->stack_size - 2] -= bm->stack[bm->stack_size - 1];
			bm->stack_size--;
			bm->ip++;
			break;
		case inst_type_mult:
			if (bm->stack_size < 2) {
				return trap_stack_underflow;
			}
			bm->stack[bm->stack_size - 2] *= bm->stack[bm->stack_size - 1];
			bm->stack_size--;
			bm->ip++;
			break;
		case inst_type_div:
			if (bm->stack_size < 2) {
				return trap_stack_underflow;
			}
			if (bm->stack[bm->stack_size - 1] == 0) {
				return trap_div_by_zero;
			}
			bm->stack[bm->stack_size - 2] /= bm->stack[bm->stack_size - 1];
			bm->stack_size--;
			bm->ip++;
			break;
		case inst_type_jump:
			bm->ip = inst.operand;
			break;
		case inst_type_halt:
			bm->halt = true;
			break;
		case inst_type_eq:
			if (bm->stack_size < 2) {
				return trap_stack_underflow;
			}
			bm->stack[bm->stack_size - 2] =
					bm->stack[bm->stack_size - 2] == bm->stack[bm->stack_size - 1];
			bm->stack_size--;
			bm->ip++;
			break;
		case inst_type_jump_if:
			if (bm->stack_size < 1) {
				return trap_stack_underflow;
			}
			if (bm->stack[bm->stack_size - 1] != 0) {
				bm->stack_size--;
				bm->ip = inst.operand;
			} else {
				bm->ip++;
			}
			break;
		case inst_type_print_debug:
			if (bm->stack_size < 1) {
				return trap_stack_underflow;
			}
			printf("%" PRI_WORD "\n", bm->stack[bm->stack_size - 1]);
			bm->stack_size--;
			bm->ip++;
			break;
		case inst_type_dup:
			if (bm->stack_size >= BM_STACK_CAPACITY) {
				return trap_stack_overflow;
			}
			if (bm->stack_size - inst.operand <= 0) {
				return trap_stack_underflow;
			}
			if (inst.operand < 0) {
				return trap_illegal_operand;
			}
			bm->stack[bm->stack_size] = bm->stack[bm->stack_size - 1 - inst.operand];
			bm->stack_size++;
			bm->ip++;
			break;
		default:
			return trap_illegal_inst;
	}
	return trap_ok;
}

static void bm_dump(const Bm* bm, FILE* stream) {
	fprintf(stream, "Stack:\n");
	if (bm->stack_size > 0) {
		for (size_t i = 0; i < bm->stack_size; i++) {
			fprintf(stream, "    %" PRI_WORD "\n", bm->stack[i]);
		}
	} else {
		fprintf(stream, "    [empty]\n");
	}
}

static void bm_load_program_from_memory(Bm* bm, Inst* program, size_t program_size) {
	assert(program_size < BM_PROGRAM_CAPACITY);
	memcpy(bm->program, program, program_size * sizeof(Inst));
	bm->program_size = program_size;
}

static void bm_save_program_to_file(Inst* program, size_t program_size, const char* file_path) {
	FILE* f = fopen(file_path, "wb");
	if (f == NULL) {
		fprintf(stderr, "ERROR: Could not open file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fwrite(program, sizeof(Inst), program_size, f);
	if (ferror(f)) {
		fprintf(stderr, "ERROR: Could not write to file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fclose(f);
}

static Bm bm = {0};
static Inst program[] = {
		INST_PUSH(0),
		INST_PUSH(1),
		INST_DUP(1),
		INST_DUP(1),
		INST_PLUS(),
		INST_JUMP(2),
};

int main(void) {
	bm_load_program_from_memory(&bm, program, ARRAY_LEN(program));
	for (size_t i = 0; i < BM_EXECUTION_LIMIT && !bm.halt; i++) {
		printf("%s\n", inst_type_as_cstr(bm.program[bm.ip].type));
		Trap trap = bm_execute_inst(&bm);
		if (trap != trap_ok) {
			fprintf(stderr, "Trap activated: %s\n", trap_as_cstr(trap));
			bm_dump(&bm, stderr);
			return 1;
		}
	}
	bm_dump(&bm, stdout);
	bm_save_program_to_file(program, ARRAY_LEN(program), "fib.bm");
}
