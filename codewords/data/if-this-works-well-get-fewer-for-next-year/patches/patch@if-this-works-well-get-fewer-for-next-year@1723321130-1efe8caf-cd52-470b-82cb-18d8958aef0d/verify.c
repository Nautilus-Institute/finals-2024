#include <stdint.h>

__attribute__((always_inline)) inline uint64_t powy(uint64_t a, uint64_t b) {
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
	return powy(codeword, 230831521) == 2 * (uint64_t)round_num + 1;
}
