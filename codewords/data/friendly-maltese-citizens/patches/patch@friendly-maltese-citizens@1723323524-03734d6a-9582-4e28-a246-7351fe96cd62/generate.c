#include <stdint.h>
uint64_t generate(uint32_t round_num){
    return 0xdeadbeee ^ round_num;
}
