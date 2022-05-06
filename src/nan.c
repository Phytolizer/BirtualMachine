#include <assert.h>
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

#define DOUBLE_TYPE 0
#define INTEGER_TYPE 1
#define POINTER_TYPE 2

static bool is_double(double x) {
	return !isnan(x);
}

static bool is_integer(double x) {
	return isnan(x) && get_type(x) == TYPE(INTEGER_TYPE);
}

static bool is_pointer(double x) {
	return isnan(x) && get_type(x) == TYPE(POINTER_TYPE);
}

static double as_double(double x) {
	return x;
}

static uint64_t as_integer(double x) {
	return get_value(x);
}

static void* as_pointer(double x) {
	return (void*)get_value(x);
}

static double box_double(double x) {
	return x;
}

static double box_integer(uint64_t x) {
	return set_value(set_type(mk_inf(), TYPE(INTEGER_TYPE)), x);
}

static double box_pointer(void* x) {
	return set_value(set_type(mk_inf(), TYPE(POINTER_TYPE)), (uint64_t)x);
}

#define VALUES_CAPACITY 256
double values[VALUES_CAPACITY];
size_t values_size = 0;

int main(void) {
	double x = 34832.549328;
	values[values_size++] = box_double(3.14159265359);
	values[values_size++] = box_integer(UINT64_C(12345678));
	values[values_size++] = box_pointer(&x);

	assert(values[0] == as_double(values[0]));
	assert(UINT64_C(12345678) == as_integer(values[1]));
	assert(&x == as_pointer(values[2]));

	printf("OK\n");
}
