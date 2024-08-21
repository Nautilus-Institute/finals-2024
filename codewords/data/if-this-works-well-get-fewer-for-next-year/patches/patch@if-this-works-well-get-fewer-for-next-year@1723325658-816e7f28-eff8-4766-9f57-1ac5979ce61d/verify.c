#include <stdint.h>

__attribute__((always_inline)) inline uint64_t powy(uint64_t a, uint64_t b) {
	uint64_t r0 = 1, r1 = a;
	for (int i = 0; i < 40; i++) {
		if ((b & 1) == 0) {
			r1 = r0 * r1;
			r0 = r0 * r0;
		} else {
			r0 = r0 * r1;
			r1 = r1 * r1;
		}
		b >>= 1;
	}
	return r0;
}

uint64_t verify(uint32_t round_num, uint64_t codeword) {
	return powy(codeword, 620484423560ULL) == 2 * (uint64_t)round_num + 1;
}
