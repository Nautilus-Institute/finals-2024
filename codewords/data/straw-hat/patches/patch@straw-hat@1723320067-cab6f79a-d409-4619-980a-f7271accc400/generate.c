#include <stdint.h>
uint64_t generate(uint32_t round_num) {
    uint64_t a = 0x6142434445464749+1 + round_num;
    return a;
}