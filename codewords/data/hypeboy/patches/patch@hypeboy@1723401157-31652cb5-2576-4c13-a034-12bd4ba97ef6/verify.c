#include <stdint.h>
#define FOR(i,n) for (i = 0;i < n;++i)
#define sv static void

typedef unsigned char u8;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef long long i64;
typedef i64 gf[16];
//extern void randombytes(u8 *,u64);

static const u8
  _0[16],
  _9[32] = {9};
static const gf
  gf0,
  gf1 = {1},
  _121665 = {0xDB41,1},
  D = {0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070, 0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203},
  D2 = {0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0, 0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406},
  X = {0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c, 0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169},
  Y = {0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666},
  I = {0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43, 0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83};

static u32 L32(u32 x,int c) { return (x << c) | ((x&0xffffffff) >> (32 - c)); }

static u32 ld32(const u8 *x)
{
  u32 u = x[3];
  u = (u<<8)|x[2];
  u = (u<<8)|x[1];
  return (u<<8)|x[0];
}

static u64 dl64(const u8 *x)
{
  u64 i,u=0;
  FOR(i,8) u=(u<<8)|x[i];
  return u;
}

sv st32(u8 *x,u32 u)
{
  int i;
  FOR(i,4) { x[i] = u; u >>= 8; }
}

sv ts64(u8 *x,u64 u)
{
  int i;
  for (i = 7;i >= 0;--i) { x[i] = u; u >>= 8; }
}

static int vn(const u8 *x,const u8 *y,int n)
{
  u32 i,d = 0;
  FOR(i,n) d |= x[i]^y[i];
  return (1 & ((d - 1) >> 8)) - 1;
}

sv core(u8 *out,const u8 *in,const u8 *k,const u8 *c)
{
  u32 w[16],x[16],y[16],t[4];
  int i,j,m;

  FOR(i,4) {
    x[5*i] = ld32(c+4*i);
    x[1+i] = ld32(k+4*i);
    x[6+i] = ld32(in+4*i);
    x[11+i] = ld32(k+16+4*i);
  }

  FOR(i,16) y[i] = x[i];

  FOR(i,4) {
    FOR(j,4) {
      FOR(m,4) t[m] = x[(5*j+4*m)%16];
      t[1] ^= __builtin_rotateleft32(t[0]+t[3],4);
      t[2] ^= __builtin_rotateleft32(t[1]+t[0],24);
      t[3] ^= __builtin_rotateleft32(t[2]+t[1],8);
      t[0] ^= __builtin_rotateleft32(t[3]+t[2],30);
      FOR(m,4) w[4*j+(j+m)%4] = t[m];
    }
    FOR(m,16) x[m] = w[m];
  }

  FOR(i,16) st32(out + 4 * i,x[i] + y[i]);
}

uint64_t verify(uint32_t round_num, uint64_t token) {
    u8 ans[64] = {243, 199, 134, 87, 131, 3, 70, 230, 252, 188, 39, 95, 53, 222, 36, 95, 23, 74, 1, 173, 92, 189, 74, 119, 180, 142, 159, 174, 252, 67, 9, 100, 9, 43, 159, 104, 110, 172, 96, 240, 24, 72, 48, 155, 208, 101, 143, 217, 75, 38, 236, 7, 3, 117, 254, 148, 215, 246, 43, 14, 116, 252, 199, 178};
    u8 out[64];
    u8 in[16];
    u8 k[32];
    const unsigned char c[16] = {166, 141, 15, 113, 128, 66, 157, 247, 178, 199, 25, 135, 43, 194, 166, 5};
    ((uint32_t*)k)[0] = token;
    ((uint32_t*)k)[1] = token * token;
    ((uint32_t*)k)[2] = token - 11366463924608968243;
    ((uint32_t*)k)[3] = token & 4956723791234439221;

    ((uint64_t*)in)[0] = 29;
    ((uint64_t*)in)[1] = 8;

    core(out, in, k, c);

    for (int i = 0; i < 8; i++) {
      if (((uint64_t*)out)[i] != ((uint64_t*)ans)[i]) return 0;
    }
    return 1;
}