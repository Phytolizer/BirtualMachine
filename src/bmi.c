#include "./bm.c"

static Bm bm = {0};

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: ./bmi <input.bm>\n");
		fprintf(stderr, "ERROR: expected input\n");
		exit(1);
	}
	bm_load_program_from_file(&bm, argv[1]);
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
