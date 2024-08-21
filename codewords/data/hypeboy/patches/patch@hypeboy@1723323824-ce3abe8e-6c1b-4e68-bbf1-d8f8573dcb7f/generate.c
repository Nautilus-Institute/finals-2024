#include <stdint.h>
uint64_t mod_exp(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}
uint64_t n = 3050666093; 
uint64_t d = 994061221;
uint64_t generate(uint32_t round_num) {
    return mod_exp(round_num, d, n);
}
