#include <stdint.h>

uint64_t hash(uint64_t x){
    for(int i=0;i<10;++i){
        switch(x&15){
            case 0: x *= x; break;
            case 1: x ^= x>>21; break;
            case 2: x ^= x<<13; break;
            case 3: x ^= 0xdeadbeef; break;
            case 4: x ^= x<<15; break;
            case 5: x ^= x * 0xcafebabe; break;
            case 6: x ^= x - 0x13371337; break;
            case 7: x ^= x + 0xcafebabe; break;
            case 8: x ^= (x >> 24) ^ (x << 36); break;
            case 9: x *= x; break;
            case 10: x ^= x>>27; break;
            case 11: x ^= x<<12; break;
            case 12: x ^= 0xd10c4; break;
            case 13: x ^= x>>12; break;
            case 14: x ^= x * 0xbbccddaa; break;
            case 15: x ^= x - 0xbbccddaa; break;
        }
    }
    return x;
}

uint64_t generate(uint32_t round_num){
    return 12529665227699709332ULL;
}
