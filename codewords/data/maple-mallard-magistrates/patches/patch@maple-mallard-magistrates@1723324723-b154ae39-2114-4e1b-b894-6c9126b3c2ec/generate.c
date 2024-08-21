#include <stdint.h>
uint64_t generate(uint32_t round_num) {
    uint64_t x = ((round_num + 98914286405326861ull) ^ 2990610511830858885ull) + 1124530174013191545ull;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
}