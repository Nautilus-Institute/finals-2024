#include <stdint.h>

#define shift(w, k) ((w << k) | w >> (32 - k))

uint64_t verify(uint32_t round_num, uint64_t token) {
    uint32_t x, y, tmp;

    uint32_t k0 = token;
    uint32_t k1 = token >> 32;

    x = 0xcafebabe;
    y = 0xdeadbeef;

    for (int i = 0; i < 26; ++i)
    {
        tmp = x;
        x = y ^ (shift(x, 1) & shift(x, 8)) ^ shift(x, 2) ^ k0;
        y = tmp;

        k0 *= k1;
        k0 ^= 0xBAADF00D;
    }

    uint64_t c = (((uint64_t) x) << 32) | ((uint64_t) y);

    return c == 0xa39e4f21c27d3565;
}
