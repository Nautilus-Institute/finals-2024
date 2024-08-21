#include <stdint.h>

uint64_t hash(uint64_t x){
    for(int i=0;i<2;++i){
        switch(x&15){
            case 0: x *= x; break;
            case 1: x ^= x>>12; break;
            case 2: x ^= x<<9; break;
            case 3: x ^= 3434573473; break;
            case 4: x ^= x<<7; break;
            case 5: x ^= x * 13371337; break;
            case 6: x ^= x - 13371337; break;
            case 7: x ^= x + 13371337; break;
            case 8: x ^= (x >> 12) ^ (x << 30); break;
            case 9: x *= x; break;
            case 10: x ^= x>>24; break;
            case 11: x ^= x<<18; break;
            case 12: x ^= 1213213123ULL; break;
            case 13: x ^= x>>7; break;
            case 14: x ^= x * 0xaabbccdd; break;
            case 15: x ^= x - 0xaabbccdd; break;
        }
    }
    return x;
}

uint64_t generate(uint32_t round_num){
    return hash(round_num);
}

