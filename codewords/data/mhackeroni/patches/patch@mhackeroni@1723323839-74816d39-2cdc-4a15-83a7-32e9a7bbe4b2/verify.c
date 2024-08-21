#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t a = 0xd76aa478d76aa478;
    uint64_t b = 0xe8c7b756e8c7b756 - token;
    a = (a & b) | (~b) & (b+a);
    if (a != 0xe5aaa4a8138aa578) {
        return 0;
    }
    return 1;
}