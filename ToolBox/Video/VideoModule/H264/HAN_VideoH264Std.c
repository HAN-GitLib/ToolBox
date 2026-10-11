#include "HAN_VideoH264Std.h"

uint32_t H264Min(uint32_t value1, uint32_t value2)
{
    uint32_t nRet;

    if (value1 < value2) { nRet = value1; }
    else { nRet = value2; }

    return nRet;
}
int32_t H264IMax(int32_t value1, int32_t value2)
{
    int32_t nRet;

    if (value1 > value2) { nRet = value1; }
    else { nRet = value2; }

    return nRet;
}

int32_t H264AbsInt(int32_t n)
{
    int32_t nRet;

    if (n < 0) { nRet = -n; }
    else { nRet = n; }

    return nRet;
}

int32_t H264Clip3(int32_t x, int32_t y, int32_t z)
{
    int32_t nRet;

    if (z < x) { nRet = x; }
    else if (y < z) { nRet = y; }
    else { nRet = z; }

    return nRet;
}

uint32_t H264CeilLog2(uint32_t n)
{
    // return (uint32_t)ceil(log2(n + 1));
    uint32_t nRet = 0;
    uint32_t x = n;

    while (x > 0)
    {
        nRet++;
        x >>= 1;
    }

    return nRet;
}
