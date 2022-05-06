#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void print_bits(uint8_t* bytes, size_t bytesSize) {
	for (int i = (int)bytesSize - 1; i >= 0; i--) {
		uint8_t byte = bytes[i];
		for (size_t j = 0; j < 8; j++) {
			printf("%d", (byte & (1 << (7 - j))) != 0);
		}
		printf(" ");
	}
	printf("\n");
}

#define EXP_MASK (((UINT64_C(1) << UINT64_C(11)) - UINT16_C(1)) << UINT64_C(52))
#define FRACTION_MASK ((UINT64_C(1) << UINT64_C(52)) - UINT64_C(1))
#define SIGN_MASK (UINT64_C(1) << UINT64_C(63))
#define TYPE_MASK (((UINT64_C(1) << UINT64_C(4)) - UINT64_C(1)) << UINT64_C(48))
#define VALUE_MASK ((UINT64_C(1) << UINT64_C(48)) - UINT64_C(1))

#define TYPE(n) ((UINT64_C(1) << UINT64_C(3)) + n)

static double mk_inf(void) {
	uint64_t y = EXP_MASK;
	return *(double*)&y;
}

static double set_type(double x, uint64_t type) {
	uint64_t y = *(uint64_t*)&x;
	y = (y & (~TYPE_MASK)) | ((TYPE_MASK >> UINT64_C(48)) & type) << UINT64_C(48);
	return *(double*)&y;
}

static uint64_t get_type(double x) {
	uint64_t y = *(uint64_t*)&x;
	return (y & TYPE_MASK) >> UINT64_C(48);
}

static double set_value(double x, uint64_t value) {
	uint64_t y = *(uint64_t*)&x;
	y = (y & ~VALUE_MASK) | (VALUE_MASK & value);
	return *(double*)&y;
}

static uint64_t get_value(double x) {
	uint64_t y = *(uint64_t*)&x;
	return y & VALUE_MASK;
}

static bool is_nan(double x) {
	uint64_t y = *(uint64_t*)&x;
	return ((y & EXP_MASK) == EXP_MASK) && (y & FRACTION_MASK) != 0;
}

static bool is_inf(double x) {
	uint64_t y = *(uint64_t*)&x;
	return ((y & EXP_MASK) == EXP_MASK) && (y & FRACTION_MASK) != 0 && (y & SIGN_MASK) == 0;
}

#define INSPECT_VALUE(type, value, label) \
	do { \
		type name = (value); \
		printf("%s = \n    ", label); \
		print_bits((uint8_t*)&name, sizeof(name)); \
		printf("    is_nan = %s\n", is_nan(name) ? "yes" : "no"); \
		printf("    isnan = %s\n", isnan(name) ? "yes" : "no"); \
	} while (false)

int main(void) {
	for (uint64_t x = 0; x < UINT64_C(8); x++) {
		INSPECT_VALUE(double, set_value(set_type(mk_inf(), TYPE(x)), UINT64_C(1234567)), "0");
	}
	return 0;
}
