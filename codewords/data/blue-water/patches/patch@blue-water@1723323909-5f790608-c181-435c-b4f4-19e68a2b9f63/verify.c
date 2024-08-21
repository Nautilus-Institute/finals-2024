#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t token) {
    token = ((token >> 19) ^ (token << 45)) ^ ((token >> 51) ^ (token << 13)) ^ ((token >> 46) ^ (token << 18));
    token ^= 0x37d59d6a210541ffuLL;
    token = ((token >> 17) ^ (token << 47)) ^ ((token >> 31) ^ (token << 33)) ^ ((token >> 55) ^ (token << 9));
    token ^= 0xee9c308b0f4109cbuLL;
    token = ((token >> 18) ^ (token << 46)) ^ ((token >> 21) ^ (token << 43)) ^ ((token >> 47) ^ (token << 17));
    token ^= 0x988e1d0c3a3a427buLL;
    token = ((token >> 15) ^ (token << 49)) ^ ((token >> 52) ^ (token << 12)) ^ ((token >> 3) ^ (token << 61));
    token ^= 0x4fbf44ffcadadac6uLL;
    token = ((token >> 53) ^ (token << 11)) ^ ((token >> 57) ^ (token << 7)) ^ ((token >> 42) ^ (token << 22));
    token ^= 0x1d57f55bfa8037b6uLL;
    return token == round_num;
}
