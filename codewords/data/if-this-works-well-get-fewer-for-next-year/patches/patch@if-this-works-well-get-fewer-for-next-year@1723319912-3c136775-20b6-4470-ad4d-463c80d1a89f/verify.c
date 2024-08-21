#include <stdint.h>

uint64_t verify(uint32_t round_num, uint64_t codeword) {
    return codeword - 1234 == round_num;
}
