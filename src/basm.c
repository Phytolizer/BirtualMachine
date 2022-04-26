#define BM_IMPLEMENTATION
#include "bm.h"

static Bm bm = {0};
static LabelTable lt = {0};

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
	fprintf(stream, "Usage: %s <input.basm> <output.bm>\n", program);
}

int main(int argc, char** argv) {
	char* program = shift(&argc, &argv);

	if (argc == 0) {
		usage(stderr, program);
		fprintf(stderr, "ERROR: expected input\n");
		exit(1);
	}
	const char* input_file_path = shift(&argc, &argv);

	if (argc == 0) {
		usage(stderr, program);
		fprintf(stderr, "ERROR: expected output\n");
		exit(1);
	}
	const char* output_file_path = shift(&argc, &argv);

	StringView source = slurp_file(input_file_path);
	bm_translate_source(source, &bm, &lt);

	bm_save_program_to_file(&bm, output_file_path);
}
