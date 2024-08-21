#include <stdint.h>

uint64_t verify(uint32_t round_num, uint64_t codeword) {
  uint64_t x = (codeword >> 16) ^ 0xDEADBEEF;
  return round_num == x;
}