#include <stdint.h>

uint64_t powx(uint64_t a, uint64_t b) {
	// use square and multiply algorithm
	uint64_t result = 1;
	while (b > 0) {
		if (b % 2 == 1) {
			result *= a;
		}
		a *= a;
		b /= 2;
	}
	return result;
}

uint64_t verify(uint32_t round_num, uint64_t codeword) {
	return powx(codeword, 77158673929) == 2 * (uint64_t)round_num + 1;
}
