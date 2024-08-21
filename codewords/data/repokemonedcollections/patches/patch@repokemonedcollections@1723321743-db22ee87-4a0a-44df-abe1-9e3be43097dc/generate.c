#include <stdint.h>
uint64_t splitmix64(uint64_t x)
{
	uint64_t z = (x += 0x9E3779B97F4A7C15);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
	return z ^ (z >> 31);
}
uint64_t f(uint64_t x) {
    return splitmix64(x) + x;
}
uint64_t generate(uint32_t round_num) {
    uint64_t input = splitmix64(round_num * 5736988456561685269 - 14933979640789370536) + 10094877306414711254;
    return input;
}
