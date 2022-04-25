#include <assert.h>
#include <ctype.h>
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
	X(nop) \
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

#define INST_NOP() \
	{ .type = inst_type_nop }
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
		case inst_type_nop:
			bm->ip++;
			break;
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

static void bm_load_program_from_file(Bm* bm, const char* file_path) {
	FILE* f = fopen(file_path, "rb");
	if (f == NULL) {
		fprintf(stderr, "ERROR: Could not open file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	if (fseek(f, 0, SEEK_END) < 0) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	long m = ftell(f);
	if (m < 0) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	assert(m % sizeof(bm->program[0]) == 0);
	assert((size_t)m <= BM_PROGRAM_CAPACITY * sizeof(bm->program[0]));

	if (fseek(f, 0, SEEK_SET) < 0) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	bm->program_size = fread(bm->program, sizeof(bm->program[0]), m / sizeof(bm->program[0]), f);

	if (ferror(f)) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fclose(f);
}

char source_code[] = "push 0\n"
					 "push 1\n"
					 "dup 1\n"
					 "dup 1\n"
					 "plus\n"
					 "jmp 2\n";

typedef struct {
	size_t count;
	const char* data;
} StringView;

static StringView cstr_as_sv(const char* cstr) {
	return (StringView){
			.count = strlen(cstr),
			.data = cstr,
	};
}

static StringView sv_trim_left(StringView sv) {
	size_t i = 0;
	while (i < sv.count && isspace(sv.data[i])) {
		i++;
	}
	return (StringView){
			.count = sv.count - i,
			.data = sv.data + i,
	};
}

static StringView sv_trim_right(StringView sv) {
	size_t i = 0;
	while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
		i++;
	}
	return (StringView){
			.count = sv.count - i,
			.data = sv.data,
	};
}

static StringView sv_trim(StringView sv) {
	return sv_trim_right(sv_trim_left(sv));
}

static StringView sv_chop_by_delim(StringView* sv, char delim) {
	size_t i = 0;
	while (i < sv->count && sv->data[i] != delim) {
		i++;
	}
	StringView result = {
			.count = i,
			.data = sv->data,
	};
	if (i < sv->count) {
		sv->count -= i + 1;
		sv->data += i + 1;
	} else {
		sv->count -= i;
		sv->data += i;
	}
	return result;
}

static bool sv_eq(StringView a, StringView b) {
	if (a.count != b.count) {
		return false;
	}
	return a.count == 0 || memcmp(a.data, b.data, a.count) == 0;
}

static int sv_to_int(StringView sv) {
	int result = 0;
	for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); i++) {
		result = result * 10 + sv.data[i] - '0';
	}
	return result;
}

static Inst bm_translate_line(StringView line) {
	line = sv_trim_left(line);
	StringView inst_name = sv_chop_by_delim(&line, ' ');
	if (sv_eq(inst_name, cstr_as_sv("push"))) {
		line = sv_trim_left(line);
		int operand = sv_to_int(sv_trim_right(line));
		return (Inst){.type = inst_type_push, .operand = operand};
	} else if (sv_eq(inst_name, cstr_as_sv("dup"))) {
		line = sv_trim_left(line);
		int operand = sv_to_int(sv_trim_right(line));
		return (Inst){.type = inst_type_dup, .operand = operand};
	} else if (sv_eq(inst_name, cstr_as_sv("plus"))) {
		return (Inst){.type = inst_type_plus};
	} else if (sv_eq(inst_name, cstr_as_sv("jmp"))) {
		line = sv_trim_left(line);
		int operand = sv_to_int(sv_trim_right(line));
		return (Inst){.type = inst_type_dup, .operand = operand};
	} else {
		fprintf(stderr, "ERROR: unknown instruction `%.*s`\n", (int)inst_name.count,
				inst_name.data);
		exit(1);
	}
}

static size_t bm_translate_source(StringView source, Inst* program, size_t program_capacity) {
	size_t program_size = 0;
	while (source.count > 0) {
		assert(program_size < program_capacity);
		StringView line = sv_trim(sv_chop_by_delim(&source, '\n'));
		if (line.count > 0) {
			program[program_size++] = bm_translate_line(line);
		}
	}
	return program_size;
}

static StringView slurp_file(const char* file_path) {
	FILE* f = fopen(file_path, "r");
	if (f == NULL) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	if (fseek(f, 0, SEEK_END) < 0) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	long m = ftell(f);
	if (m < 0) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	char* buffer = malloc(m);
	if (buffer == NULL) {
		fprintf(stderr, "ERROR: Could not allocate memory for file: %s\n", strerror(errno));
		exit(1);
	}

	if (fseek(f, 0, SEEK_SET) < 0) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fread(buffer, 1, m, f);
	if (ferror(f)) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fclose(f);
	return (StringView){.count = m, .data = buffer};
}

static Bm bm = {0};

int main(int argc, char** argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: ./bm <input.ebasm> <output.bm>\n");
		fprintf(stderr, "ERROR: expected input and output\n");
		exit(1);
	}
	const char* input_file_path = argv[1];
	const char* output_file_path = argv[2];

	StringView source = slurp_file(input_file_path);
	bm.program_size = bm_translate_source(source, bm.program, BM_PROGRAM_CAPACITY);

	bm_save_program_to_file(bm.program, bm.program_size, output_file_path);
}

static int __attribute__((unused)) main2(void) {
	bm_translate_source(cstr_as_sv(source_code), bm.program, BM_PROGRAM_CAPACITY);
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
	return 0;
}
