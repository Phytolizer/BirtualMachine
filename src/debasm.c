#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: ./debasm <input.bm>\n");
		fprintf(stderr, "ERROR: no input provided\n");
		exit(1);
	}

	for (size_t i = 0; i < bm.program_size; i++) {
		switch (bm.program[i].type) {}
	}
}
