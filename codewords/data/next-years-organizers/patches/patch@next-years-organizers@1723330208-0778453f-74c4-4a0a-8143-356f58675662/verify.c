#include <stdint.h>

const uint64_t mod = 2797000457;


uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t result = 1;
    token = token % mod;  // In case base is greater than mod

    uint64_t exp = 7;
    while (exp > 0) {
        // If exp is odd, multiply base with the result
        if (exp % 2 == 1) {
            result = (result * token) % mod;
        }
        // exp must be even now
        exp = exp >> 1;  // Divide exp by 2
        token = (token * token) % mod;  // Square the base
    }

    if (result != round_num) {
        return 0;
    }
    return 1;
}

