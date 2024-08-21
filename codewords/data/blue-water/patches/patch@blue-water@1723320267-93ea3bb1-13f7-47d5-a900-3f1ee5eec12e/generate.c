#include <stdint.h>
uint64_t generate(uint32_t round_num) {
    uint64_t a = 0x4142434445466969 + round_num*21;
    return a;
}

