#include <stdint.h>
uint64_t n = 1522806097; 
uint64_t e = 1061295217; 

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

uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t decrypted = mod_exp(token, e, n);
    if (decrypted == round_num) {
        return 1;
    } else {
        return 0;
    }
}