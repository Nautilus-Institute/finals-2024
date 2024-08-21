#include <stdint.h>
uint64_t verify(uint32_t round_num, uint64_t token) {
    uint64_t r=0x14020a57acced8b7,x,h=0x1337;
    while(token)x=token%1337,x*=r,x=x<<31|x>>33,h=h*r^x,h=h<<31|h>>33,token/=1337;
    return (h=h*r+8,h^=h>>31,h*=r,h^=h>>31,h*=r,h^=h>>31,h*=r,h==0x99061f54508b4125);
}

