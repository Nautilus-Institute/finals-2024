#include <stdint.h>

uint64_t e = 7;
uint64_t mod = 2797000457;


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

uint64_t generate(uint32_t round_num) {
    uint32_t d = 1198668343;
    return modpow(round_num, d, mod);
}
