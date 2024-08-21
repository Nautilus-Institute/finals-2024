#include <stdint.h>
uint64_t generate(uint32_t round_num) {
    uint64_t a = 0x69696969 + round_num;
    return a;
}
