#include <stdint.h>

uint64_t mul(uint32_t a, uint64_t b) {
    uint64_t o = 0;
    uint64_t n = 121575418825321ULL;
    while(a) {
        if(a & 1) {
            o += b;
            if(o >= n) o -= n;
        }
        b += b;
        if(b >= n) b -= n;
        a >>= 1;
    }
    return o;
}
uint64_t generate(uint32_t roundnr) {
    uint64_t v = mul(roundnr ^ 190, 80646719348298ULL);
    uint64_t out = 0;
    if(v & 1ULL) out |= 17592186044416ULL;
    if(v & 2ULL) out |= 8388608ULL;
    if(v & 4ULL) out |= 1ULL;
    if(v & 8ULL) out |= 1048576ULL;
    if(v & 16ULL) out |= 4194304ULL;
    if(v & 32ULL) out |= 8192ULL;
    if(v & 64ULL) out |= 4096ULL;
    if(v & 128ULL) out |= 134217728ULL;
    if(v & 256ULL) out |= 536870912ULL;
    if(v & 512ULL) out |= 137438953472ULL;
    if(v & 1024ULL) out |= 1073741824ULL;
    if(v & 2048ULL) out |= 17179869184ULL;
    if(v & 4096ULL) out |= 2147483648ULL;
    if(v & 8192ULL) out |= 35184372088832ULL;
    if(v & 16384ULL) out |= 8796093022208ULL;
    if(v & 32768ULL) out |= 274877906944ULL;
    if(v & 65536ULL) out |= 32768ULL;
    if(v & 131072ULL) out |= 2199023255552ULL;
    if(v & 262144ULL) out |= 67108864ULL;
    if(v & 524288ULL) out |= 33554432ULL;
    if(v & 1048576ULL) out |= 8589934592ULL;
    if(v & 2097152ULL) out |= 1024ULL;
    if(v & 4194304ULL) out |= 1099511627776ULL;
    if(v & 8388608ULL) out |= 512ULL;
    if(v & 16777216ULL) out |= 64ULL;
    if(v & 33554432ULL) out |= 70368744177664ULL;
    if(v & 67108864ULL) out |= 2097152ULL;
    if(v & 134217728ULL) out |= 128ULL;
    if(v & 268435456ULL) out |= 16ULL;
    if(v & 536870912ULL) out |= 8ULL;
    if(v & 1073741824ULL) out |= 32ULL;
    if(v & 2147483648ULL) out |= 524288ULL;
    if(v & 4294967296ULL) out |= 549755813888ULL;
    if(v & 8589934592ULL) out |= 16384ULL;
    if(v & 17179869184ULL) out |= 131072ULL;
    if(v & 34359738368ULL) out |= 2048ULL;
    if(v & 68719476736ULL) out |= 4ULL;
    if(v & 137438953472ULL) out |= 4398046511104ULL;
    if(v & 274877906944ULL) out |= 140737488355328ULL;
    if(v & 549755813888ULL) out |= 268435456ULL;
    if(v & 1099511627776ULL) out |= 2ULL;
    if(v & 2199023255552ULL) out |= 16777216ULL;
    if(v & 4398046511104ULL) out |= 68719476736ULL;
    if(v & 8796093022208ULL) out |= 4294967296ULL;
    if(v & 17592186044416ULL) out |= 34359738368ULL;
    if(v & 35184372088832ULL) out |= 262144ULL;
    if(v & 70368744177664ULL) out |= 65536ULL;
    if(v & 140737488355328ULL) out |= 256ULL;
    return out;
}
