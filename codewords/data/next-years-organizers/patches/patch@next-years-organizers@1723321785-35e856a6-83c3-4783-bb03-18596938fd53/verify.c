#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t a = 0x4182434445464749 + round_num;
    if (a != token) {
        return 0;
    }
    return 1;
}