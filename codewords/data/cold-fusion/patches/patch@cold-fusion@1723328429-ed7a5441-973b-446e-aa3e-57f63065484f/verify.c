#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t base;
    uint32_t temp;
    base = 1;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[4];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[3];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[0];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[1];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[2];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[1];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[4];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[0];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[3];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[1];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[2];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[7];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[7];
    base *= temp;
    base ^= 8652140726583807411;
    base %= 1000000007;

    if (base != 678404766) return 0;

    base = 1;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[0];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[1];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[1];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[3];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[2];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[3];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[4];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[5];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[0];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[7];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[6];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[7];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[2];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[5];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[7];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[3];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[6];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;
    ((uint8_t*)&temp)[0] = ((uint8_t*)&token)[1];
    ((uint8_t*)&temp)[1] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[2] = ((uint8_t*)&token)[0];
    ((uint8_t*)&temp)[3] = ((uint8_t*)&token)[4];
    base *= temp;
    base ^= 7869653795889150952;
    base %= 1000000007;

    if (base != 620726000) return 0;

    return 1;
}
