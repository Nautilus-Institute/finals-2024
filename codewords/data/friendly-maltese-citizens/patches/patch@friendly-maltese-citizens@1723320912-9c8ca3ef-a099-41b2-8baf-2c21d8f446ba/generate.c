#include <stdint.h>
uint64_t generate(uint32_t round_num) {
    uint64_t a = 0x4142434445464748 + round_num+1;
    return a;
}