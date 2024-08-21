#include <stdint.h>

__attribute__((always_inline)) inline uint64_t powx(uint64_t a, uint64_t b) {
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

uint64_t generate(uint32_t round_num) {
	return powx(2 * (uint64_t)round_num + 1, 998929002081);
}
