#define BM_IMPLEMENTATION
#include "bm.h"

static Bm bm = {0};

static char* shift(int* argc, char*** argv) {
	if (*argc == 0) {
		return NULL;
	}
	char* arg = (*argv)[0];
	(*argv)++;
	(*argc)--;
	return arg;
}

static void usage(FILE* stream, const char* program) {
	fprintf(stream, "Usage: %s -i <input.bm> [-l <limit>]\n", program);
}

int main(int argc, char** argv) {
	const char* program = shift(&argc, &argv);
	const char* input_file_path = NULL;
	int limit = -1;

	while (argc > 0) {
		const char* flag = shift(&argc, &argv);
		if (strcmp(flag, "-i") == 0) {
			if (argc == 0) {
				usage(stderr, program);
				fprintf(stderr, "ERROR: no argument provided for flag `%s`\n", flag);
				exit(1);
			}

			input_file_path = shift(&argc, &argv);
		} else if (strcmp(flag, "-l") == 0) {
			if (argc == 0) {
				usage(stderr, program);
				fprintf(stderr, "ERROR: no argument provided for flag `%s`\n", flag);
				exit(1);
			}

			errno = 0;
			limit = atoi(shift(&argc, &argv));
		} else {
			if (argc == 0) {
				usage(stderr, program);
				fprintf(stderr, "ERROR: unknown flag %s\n", flag);
				exit(1);
			}
		}
	}

	if (input_file_path == NULL) {
		usage(stderr, program);
		fprintf(stderr, "ERROR: no input provided\n");
		exit(1);
	}

	bm_load_program_from_file(&bm, input_file_path);
	Trap trap = bm_execute_program(&bm, limit);
	bm_dump(&bm, stdout);

	if (trap != trap_ok) {
		fprintf(stderr, "ERROR: %s\n", trap_as_cstr(trap));
	}
	return 0;
}
