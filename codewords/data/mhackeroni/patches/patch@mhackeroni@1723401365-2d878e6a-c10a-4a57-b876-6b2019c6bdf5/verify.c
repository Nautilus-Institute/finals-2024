#define GENERATOR 0

#include <stdint.h>

#if GENERATOR
#include <stdio.h>
#include <stdlib.h>
#endif

// uint64_t verify(uint32_t round_num, uint64_t token)
// {
//     uint64_t a = 0xd76aa478d76aa478;
//     uint64_t b = 0xe8c7b756e8c7b756 - token;
//     a = (a & b) | (~b) & (b + a);
//     if (a != 0xf950935f, 0xdbf8df88, 0x97d5d52b, 0x670ecd4dull)
//     {
//         return 0;
//     }
//     return 1;
// }

uint32_t rotateLeft(uint32_t x, uint32_t n)
{
    return (x << n) | (x >> (32 - n));
}

uint64_t verify(uint32_t round_num, uint64_t token)
{
    static uint32_t S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                           5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
                           4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                           6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
    static uint32_t K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                           0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                           0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                           0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                           0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                           0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                           0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                           0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                           0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                           0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                           0x289b7ef6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                           0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                           0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                           0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                           0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                           0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
    uint32_t buffer[4];
    uint32_t target[] = {0xf950935f, 0xdbf8df88, 0x97d5d52b, 0x670ecd4d};
    buffer[0] = 0x67452301;
    buffer[1] = 0xefcdab89;
    buffer[2] = (token >> 32);
    buffer[3] = (token & 0xffffffff);

    for (int i = 0; i < 6; i++)
    {
        uint32_t AA = buffer[0];
        uint32_t BB = buffer[1];
        uint32_t CC = buffer[2];
        uint32_t DD = buffer[3];

        uint32_t E;

        unsigned int j;

        for (unsigned int i = 0; i < 7; ++i)
        {
            switch (i / 2)
            {
            case 0:
                E = ((BB & CC) | (~BB & DD));
                j = i;
                break;
            case 1:
                E = ((BB & CC) | (DD & ~CC));
                j = ((i * 5) + 1) % 16;
                break;
            case 2:
                E = (BB ^ CC ^ DD);
                j = ((i * 3) + 5) % 16;
                break;
            default:
                E = (CC ^ (BB | ~DD));
                j = (i * 7) % 16;
                break;
            }

            uint32_t temp = DD;
            DD = CC;
            CC = BB;
            BB = BB + rotateLeft(AA + E + K[i] + 0xc33707d6, S[i]);
            AA = temp;
        }

        buffer[0] += AA;
        buffer[1] += BB;
        buffer[2] += CC;
        buffer[3] += DD;
    }
    #if GENERATOR
    for (int i = 0; i < 4; i++)
    {
        printf("%08x", buffer[i]);
    }
    #endif
    for (int i = 0; i < 4; i++)
    {
        if (buffer[i] != target[i])
        {
            return 0;
        }
    }
    return 1;
}

#if GENERATOR
int main(int argc, char *argv[])
{;
    uint64_t token = strtoull(argv[1], NULL, 16);
    verify(0, token);
    return 0;
}
#endif