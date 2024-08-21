#include <stdint.h>

const uint64_t ROUNDS = 23;

const uint64_t SHAKED = (uint64_t)(-145114034439147179);

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) (             \
	a += b, d ^= a, d = ROTL(d, 16), \
	c += d, b ^= c, b = ROTL(b, 12), \
	a += b, d ^= a, d = ROTL(d,  8), \
	c += d, b ^= c, b = ROTL(b,  7))

uint64_t shake(uint64_t in)
{
	int i;
	uint32_t x[4] = { 0xdeadbeef, in & 0xffffffff, in >> 32, 0x13371337 };

	for (i = 0; i < ROUNDS; i += 2) {
		QR(x[0], x[2], x[1], x[3]);
		QR(x[1], x[2], x[3], x[0]);
	}

	return (uint64_t)(x[0] ^ x[1]) << 32 | (x[2] ^ x[3]);
}

uint64_t verify(uint32_t round_num, uint64_t token) {
	return shake(token) == SHAKED;
}

#ifdef MAIN_FUNCTION
#include <unistd.h>
#include <stdio.h>
int main() {
	uint64_t token;
	token = 2283373611022881757;
	//getentropy(&token, sizeof token);
	printf("token = %ld\n", token);
	printf("SHAKED = %ld\n", shake(token));
	return 0;
}
#endif
