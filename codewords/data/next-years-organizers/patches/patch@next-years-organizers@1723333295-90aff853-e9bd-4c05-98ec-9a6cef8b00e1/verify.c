#include <stdint.h>

uint64_t e = 0x02E9A0BC3;
uint64_t mod = 0x83C0058D;


uint64_t modpow(uint64_t base, uint32_t exp, uint64_t mod) {
    uint64_t result = 1;
    base = base % mod;  // In case base is greater than mod

    while (exp > 0) {
        // If exp is odd, multiply base with the result
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        // exp must be even now
        exp = exp >> 1;  // Divide exp by 2
        base = (base * base) % mod;  // Square the base
    }

    return result;
}

uint64_t verify(uint32_t round_num, uint64_t token) {
    if (modpow(token, e, mod) != round_num) {
        return 0;
    }
    return 1;
}

