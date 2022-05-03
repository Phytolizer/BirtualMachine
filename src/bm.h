#ifndef BM_H_
#define BM_H_

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 69
#define LABEL_CAPACITY 1024
#define DEFERRED_OPERANDS_CAPACITY 1024
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

typedef struct {
	InstType type;
	Word operand;
} Inst;

typedef struct {
	Word stack[BM_STACK_CAPACITY];
	size_t stack_size;

	Inst program[BM_PROGRAM_CAPACITY];
	Word program_size;
	Word ip;

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

typedef struct {
	size_t count;
	const char* data;
} StringView;

typedef struct {
	StringView name;
	Word addr;
} Label;

typedef struct {
	Word addr;
	StringView label;
} DeferredOperand;

typedef struct {
	Label labels[LABEL_CAPACITY];
	size_t labels_size;
	DeferredOperand deferred_operands[DEFERRED_OPERANDS_CAPACITY];
	size_t deferred_operands_size;
} BasmContext;

const char* trap_as_cstr(Trap trap);
const char* inst_type_as_cstr(InstType type);
Trap bm_execute_inst(Bm* bm);
Trap bm_execute_program(Bm* bm, int limit);
void bm_dump(const Bm* bm, FILE* stream);
void bm_load_program_from_memory(Bm* bm, Inst* program, Word program_size);
void bm_save_program_to_file(const Bm* bm, const char* file_path);
void bm_load_program_from_file(Bm* bm, const char* file_path);
StringView cstr_as_sv(const char* cstr);
StringView sv_trim_left(StringView sv);
StringView sv_trim_right(StringView sv);
StringView sv_trim(StringView sv);
StringView sv_chop_by_delim(StringView* sv, char delim);
bool sv_eq(StringView a, StringView b);
int sv_to_int(StringView sv);
void bm_translate_source(StringView source, Bm* bm, BasmContext* basm);
Word basm_find_label_addr(const BasmContext* basm, StringView name);
void basm_push_label(BasmContext* basm, StringView name, Word addr);
void basm_push_deferred_operand(BasmContext* basm, StringView label, Word addr);
StringView slurp_file(const char* file_path);

#endif

#ifdef BM_IMPLEMENTATION

const char* trap_as_cstr(Trap trap) {
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

const char* inst_type_as_cstr(InstType type) {
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

Trap bm_execute_inst(Bm* bm) {
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

Trap bm_execute_program(Bm* bm, int limit) {
	while (limit != 0 && !bm->halt) {
		Trap trap = bm_execute_inst(bm);
		if (trap != trap_ok) {
			return trap;
		}

		if (limit > 0) {
			limit--;
		}
	}

	return trap_ok;
}

void bm_dump(const Bm* bm, FILE* stream) {
	fprintf(stream, "Stack:\n");
	if (bm->stack_size > 0) {
		for (size_t i = 0; i < bm->stack_size; i++) {
			fprintf(stream, "    %" PRI_WORD "\n", bm->stack[i]);
		}
	} else {
		fprintf(stream, "    [empty]\n");
	}
}

void bm_load_program_from_memory(Bm* bm, Inst* program, Word program_size) {
	assert(program_size < BM_PROGRAM_CAPACITY);
	memcpy(bm->program, program, program_size * sizeof(Inst));
	bm->program_size = program_size;
}

void bm_save_program_to_file(const Bm* bm, const char* file_path) {
	FILE* f = fopen(file_path, "wb");
	if (f == NULL) {
		fprintf(stderr, "ERROR: Could not open file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fwrite(bm->program, sizeof(Inst), bm->program_size, f);
	if (ferror(f)) {
		fprintf(stderr, "ERROR: Could not write to file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fclose(f);
}

void bm_load_program_from_file(Bm* bm, const char* file_path) {
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

	bm->program_size =
			(Word)fread(bm->program, sizeof(bm->program[0]), m / sizeof(bm->program[0]), f);

	if (ferror(f)) {
		fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", file_path, strerror(errno));
		exit(1);
	}

	fclose(f);
}

StringView cstr_as_sv(const char* cstr) {
	return (StringView){
			.count = strlen(cstr),
			.data = cstr,
	};
}

StringView sv_trim_left(StringView sv) {
	size_t i = 0;
	while (i < sv.count && isspace(sv.data[i])) {
		i++;
	}
	return (StringView){
			.count = sv.count - i,
			.data = sv.data + i,
	};
}

StringView sv_trim_right(StringView sv) {
	size_t i = 0;
	while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
		i++;
	}
	return (StringView){
			.count = sv.count - i,
			.data = sv.data,
	};
}

StringView sv_trim(StringView sv) {
	return sv_trim_right(sv_trim_left(sv));
}

StringView sv_chop_by_delim(StringView* sv, char delim) {
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

bool sv_eq(StringView a, StringView b) {
	if (a.count != b.count) {
		return false;
	}
	return a.count == 0 || memcmp(a.data, b.data, a.count) == 0;
}

int sv_to_int(StringView sv) {
	int result = 0;
	for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); i++) {
		result = result * 10 + sv.data[i] - '0';
	}
	return result;
}

Word basm_find_label_addr(const BasmContext* basm, StringView name) {
	for (size_t i = 0; i < basm->labels_size; i++) {
		if (sv_eq(basm->labels[i].name, name)) {
			return basm->labels[i].addr;
		}
	}
	fprintf(stderr, "ERROR: label `%.*s` does not exist\n", (int)name.count, name.data);
	exit(1);
}

void basm_push_label(BasmContext* basm, StringView name, Word addr) {
	assert(basm->labels_size < LABEL_CAPACITY);
	basm->labels[basm->labels_size++] = (Label){.name = name, .addr = addr};
}

void bm_translate_source(StringView source, Bm* bm, BasmContext* basm) {
	while (source.count > 0) {
		assert(bm->program_size < BM_PROGRAM_CAPACITY);
		StringView line = sv_trim(sv_chop_by_delim(&source, '\n'));
		if (line.count > 0 && line.data[0] != '#') {
			StringView inst_name = sv_chop_by_delim(&line, ' ');

			if (inst_name.count > 0 && inst_name.data[inst_name.count - 1] == ':') {
				StringView label = {
						.count = inst_name.count - 1,
						.data = inst_name.data,
				};
				basm_push_label(basm, label, bm->program_size);
				inst_name = sv_trim(sv_chop_by_delim(&line, ' '));
			}

			if (inst_name.count > 0) {
				StringView operand = sv_trim(sv_chop_by_delim(&line, '#'));

				if (sv_eq(inst_name, cstr_as_sv("nop"))) {
					bm->program[bm->program_size++] = (Inst){.type = inst_type_nop};
				} else if (sv_eq(inst_name, cstr_as_sv("push"))) {
					bm->program[bm->program_size++] =
							(Inst){.type = inst_type_push, .operand = sv_to_int(operand)};
				} else if (sv_eq(inst_name, cstr_as_sv("dup"))) {
					bm->program[bm->program_size++] =
							(Inst){.type = inst_type_dup, .operand = sv_to_int(operand)};
				} else if (sv_eq(inst_name, cstr_as_sv("plus"))) {
					bm->program[bm->program_size++] = (Inst){.type = inst_type_plus};
				} else if (sv_eq(inst_name, cstr_as_sv("jmp"))) {
					if (operand.count > 0 && isdigit(operand.data[0])) {
						bm->program[bm->program_size++] = (Inst){
								.type = inst_type_jump,
								.operand = sv_to_int(operand),
						};
					} else {
						basm_push_deferred_operand(basm, operand, bm->program_size);
						bm->program[bm->program_size++] = (Inst){.type = inst_type_jump};
					}
				} else {
					fprintf(stderr, "ERROR: unknown instruction `%.*s`\n", (int)inst_name.count,
							inst_name.data);
					exit(1);
				}
			}
		}
	}
	bm->program[bm->program_size++] = (Inst){.type = inst_type_halt};

	for (size_t i = 0; i < basm->deferred_operands_size; i++) {
		Word addr = basm_find_label_addr(basm, basm->deferred_operands[i].label);
		bm->program[basm->deferred_operands[i].addr].operand = addr;
	}
}

void basm_push_deferred_operand(BasmContext* basm, StringView label, Word addr) {
	assert(basm->deferred_operands_size < DEFERRED_OPERANDS_CAPACITY);
	basm->deferred_operands[basm->deferred_operands_size++] =
			(DeferredOperand){.addr = addr, .label = label};
}

StringView slurp_file(const char* file_path) {
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

#endif
