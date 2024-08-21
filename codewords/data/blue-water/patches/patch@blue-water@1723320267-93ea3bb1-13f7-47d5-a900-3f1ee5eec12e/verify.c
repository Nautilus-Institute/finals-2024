#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t a = 0x4142434445466969 + round_num*21;
    if (a != token) {
        return 0;
    }
    return 1;
}
