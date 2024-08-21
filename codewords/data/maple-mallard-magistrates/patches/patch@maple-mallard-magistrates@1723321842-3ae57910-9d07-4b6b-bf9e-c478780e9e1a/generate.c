#include <stdint.h>
uint64_t generate(uint32_t round_num) {
    uint64_t x = round_num + 31337;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;    
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;    
    return x;
}