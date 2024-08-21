#include <stdint.h>

uint64_t generate(uint32_t round_num) {
  return (round_num ^ 0xDEADBEEFll) << 16;
}
