#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: ./debasm <input.bm>\n");
		fprintf(stderr, "ERROR: no input provided\n");
		exit(1);
	}

	const char* input_file_path = argv[1];

	bm_load_program_from_file(&bm, input_file_path);

	for (size_t i = 0; i < bm.program_size; i++) {
		switch (bm.program[i].type) {
			case inst_type_nop:
				printf("nop\n");
				break;
			case inst_type_push:
				printf("push %" PRI_WORD "\n", bm.program[i].operand);
				break;
			case inst_type_plus:
				printf("plus\n");
				break;
			case inst_type_minus:
				printf("minus\n");
				break;
			case inst_type_mult:
				printf("mult\n");
				break;
			case inst_type_div:
				printf("div\n");
				break;
			case inst_type_jump:
				printf("jmp %" PRI_WORD "\n", bm.program[i].operand);
				break;
			case inst_type_jump_if:
				printf("jmp_if %" PRI_WORD "\n", bm.program[i].operand);
				break;
			case inst_type_eq:
				printf("eq\n");
				break;
			case inst_type_halt:
				break;
			case inst_type_print_debug:
				printf("print_debug\n");
				break;
			case inst_type_dup:
				printf("dup %" PRI_WORD "\n", bm.program[i].operand);
				break;
		}
	}
}
