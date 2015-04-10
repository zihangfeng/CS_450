#include "450UtilsUDP.h"


uint16_t calcCheckSum(void* addr, int nBytes)
{
uint16_t *addr16 = (uint16_t*)addr;
uint8_t *addr8 = (uint8_t*)addr;

uint32_t sum = 0, max16 = 0xffff;

int n;
for(n = 0; n < nBytes/2; n++)
{
        sum += addr16[n];
        if(sum > max16)
        sum -= max16;
}

if((nBytes % 2) == 1)
{
        sum += addr8[(nBytes - 1)];
        if(sum > max16)
        sum -= max16;
}

return (~sum);
}

