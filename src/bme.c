#define BM_IMPLEMENTATION
#include "bm.h"

static Bm bm = {0};

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: ./bme <input.bm>\n");
		fprintf(stderr, "ERROR: expected input\n");
		exit(1);
	}
	bm_load_program_from_file(&bm, argv[1]);
	Trap trap = bm_execute_program(&bm, 69);
	bm_dump(&bm, stdout);

	if (trap != trap_ok) {
		fprintf(stderr, "ERROR: %s\n", trap_as_cstr(trap));
	}
	return 0;
}
