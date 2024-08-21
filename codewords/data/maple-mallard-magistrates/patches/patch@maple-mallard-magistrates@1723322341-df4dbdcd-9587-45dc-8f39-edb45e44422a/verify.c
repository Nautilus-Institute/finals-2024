#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t codeword) {
   uint64_t x = ((round_num + 3810962295908534551) ^ 9366086091784492872) + 1363024386151976883;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    x ^= codeword;
    x ^= 1;
    return x;
}