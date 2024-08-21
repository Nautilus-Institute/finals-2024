#include <stddef.h>
#include <stdint.h>

#define MAX_ROUNDS 256

uint64_t generate(uint32_t round_num) {
	return ((uint64_t)(round_num % MAX_ROUNDS) * 0xdeadbeef) ^
	       0x8763314287633142ull;
}
