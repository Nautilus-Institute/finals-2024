#include <stdint.h>
uint32_t u32(uint64_t x) {
    return (uint32_t)x;
}

uint32_t not32(uint32_t x) {
    return ~x;
}

uint32_t __rol32__(uint32_t x, uint32_t y) {
    uint32_t hi = x << y;
    uint32_t lo = x >> (32-y);
    return hi | lo;
}

uint64_t __rol64__(uint64_t x, uint64_t y) {
    uint64_t hi = x << y;
    uint64_t lo = x >> (64-y);
    return hi | lo;
}

uint64_t verify(uint32_t round_num, uint64_t codeword) {
    uint32_t v1 = u32(codeword);
    uint32_t v2 = codeword >> 32;
    uint32_t v3 = 0x114514;
    uint32_t v4 = 0x1919810;

    for(int i = 0; i < 4; i++){

        uint32_t v5 = u32(__rol32__(u32(((not32(v3) & v2) | (v1 & v3)) + v4 - 0x4458a754), 9) + v3);
        uint32_t v6 = not32(__rol32__(u32(((u32(v3) & v2) | (v1 & v3)) + v4 - 0x78925a55), 5) + v3);

        uint32_t v7 = u32(__rol32__(u32(((u32(v3) & v5) | (v6 & v3)) + v4 - 0xccd89fa4), 12) + v5);
        uint32_t v8 = u32(__rol32__(u32(((not32(v3) & v6) | (v7 & v3)) + v4 - 0x123488aa), 11) + v6);

        uint32_t v9 = not32(__rol32__(u32(((u32(v7) & v5) | (v6 & v8)) + v4 - 0x95ac78e1), 12) + v8);
        uint32_t v10 = u32(__rol32__(u32(((u32(v8) & v6) | (v7 & v9)) + v4 - 0x70a5921b), 11) + v9);

        uint32_t v11 = u32(__rol32__(u32(((u32(v7) & v10) | (v9 & v8)) + v4 - 0x9154a8e0), 12) + v7);
        uint32_t v12 = u32(__rol32__(u32(((u32(v8) & v11) | (v10 & v9)) + v4 - 0x34845c3e), 11) + v10);

        v1 = u32(v1 + v12);
        v2 = u32(v3 + v11);
        v3 = u32(v2 | v10);
        v4 = u32(v4 + v12);
    }

    return (v3 == 4020714815u) && (v4 == 1503034324u);
}