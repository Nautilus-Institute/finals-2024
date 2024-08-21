#include <stdint.h>
uint64_t verify(uint32_t roundnr, uint64_t s) {
    uint64_t a = 0;
    uint64_t n = 121575418825321;
    if(s & 1ULL) { a += 26497798829612ULL; if(a >= n) a -= n; }
    if(s & 2ULL) { a += 44390603843058ULL; if(a >= n) a -= n; }
    if(s & 4ULL) { a += 109152904212347ULL; if(a >= n) a -= n; }
    if(s & 8ULL) { a += 58791042598101ULL; if(a >= n) a -= n; }
    if(s & 16ULL) { a += 90183230711711ULL; if(a >= n) a -= n; }
    if(s & 32ULL) { a += 117582085196202ULL; if(a >= n) a -= n; }
    if(s & 64ULL) { a += 74022625008725ULL; if(a >= n) a -= n; }
    if(s & 128ULL) { a += 105879324768516ULL; if(a >= n) a -= n; }
    if(s & 256ULL) { a += 89528025946658ULL; if(a >= n) a -= n; }
    if(s & 512ULL) { a += 97799021917023ULL; if(a >= n) a -= n; }
    if(s & 1024ULL) { a += 54843610185586ULL; if(a >= n) a -= n; }
    if(s & 2048ULL) { a += 115364161518834ULL; if(a >= n) a -= n; }
    if(s & 4096ULL) { a += 59238524797829ULL; if(a >= n) a -= n; }
    if(s & 8192ULL) { a += 90406971811575ULL; if(a >= n) a -= n; }
    if(s & 16384ULL) { a += 89628749792369ULL; if(a >= n) a -= n; }
    if(s & 32768ULL) { a += 115690817967038ULL; if(a >= n) a -= n; }
    if(s & 65536ULL) { a += 44764012973329ULL; if(a >= n) a -= n; }
    if(s & 131072ULL) { a += 57682080759417ULL; if(a >= n) a -= n; }
    if(s & 262144ULL) { a += 83169715899325ULL; if(a >= n) a -= n; }
    if(s & 524288ULL) { a += 113588751567083ULL; if(a >= n) a -= n; }
    if(s & 1048576ULL) { a += 52995597659224ULL; if(a >= n) a -= n; }
    if(s & 2097152ULL) { a += 52939662384258ULL; if(a >= n) a -= n; }
    if(s & 4194304ULL) { a += 105991195318448ULL; if(a >= n) a -= n; }
    if(s & 8388608ULL) { a += 13248899414806ULL; if(a >= n) a -= n; }
    if(s & 16777216ULL) { a += 88781207686116ULL; if(a >= n) a -= n; }
    if(s & 33554432ULL) { a += 74498611959057ULL; if(a >= n) a -= n; }
    if(s & 67108864ULL) { a += 98037015392189ULL; if(a >= n) a -= n; }
    if(s & 134217728ULL) { a += 118477049595658ULL; if(a >= n) a -= n; }
    if(s & 268435456ULL) { a += 22195301921529ULL; if(a >= n) a -= n; }
    if(s & 536870912ULL) { a += 115378680365995ULL; if(a >= n) a -= n; }
    if(s & 1073741824ULL) { a += 96788464988017ULL; if(a >= n) a -= n; }
    if(s & 2147483648ULL) { a += 22427603476105ULL; if(a >= n) a -= n; }
    if(s & 4294967296ULL) { a += 111973993093822ULL; if(a >= n) a -= n; }
    if(s & 8589934592ULL) { a += 27421805092793ULL; if(a >= n) a -= n; }
    if(s & 17179869184ULL) { a += 72001511150713ULL; if(a >= n) a -= n; }
    if(s & 34359738368ULL) { a += 102372567362323ULL; if(a >= n) a -= n; }
    if(s & 68719476736ULL) { a += 55986996546911ULL; if(a >= n) a -= n; }
    if(s & 137438953472ULL) { a += 109181941906669ULL; if(a >= n) a -= n; }
    if(s & 274877906944ULL) { a += 57845408983519ULL; if(a >= n) a -= n; }
    if(s & 549755813888ULL) { a += 105602084308845ULL; if(a >= n) a -= n; }
    if(s & 1099511627776ULL) { a += 109687220371172ULL; if(a >= n) a -= n; }
    if(s & 2199023255552ULL) { a += 109806217108755ULL; if(a >= n) a -= n; }
    if(s & 4398046511104ULL) { a += 96730389599373ULL; if(a >= n) a -= n; }
    if(s & 8796093022208ULL) { a += 89710413904420ULL; if(a >= n) a -= n; }
    if(s & 17592186044416ULL) { a += 6624449707403ULL; if(a >= n) a -= n; }
    if(s & 35184372088832ULL) { a += 44855206952210ULL; if(a >= n) a -= n; }
    if(s & 70368744177664ULL) { a += 26469831192129ULL; if(a >= n) a -= n; }
    if(s & 140737488355328ULL) { a += 71885360373425ULL; if(a >= n) a -= n; }
    return a == (roundnr ^ 190ULL);
}
