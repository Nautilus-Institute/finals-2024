#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t codeword){
    if (0xdeadbeee != (codeword ^ round_num)){
        return 0;
    }
    return 1;
}
