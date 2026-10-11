#include "HAN_VideoH264ReadBit.h"

VIDEOMP4READBIT H264InitReadBit(const uint8_t* pData, HANSIZE nLen)
{
    VIDEOMP4READBIT rueRet = {
        .pData = pData,
        .nDataLen = nLen,
        .cData = pData[0],
        .iByte = 0,
        .iBit = 0,
    };

    return rueRet;
}

uint8_t H264ReadBit(PVIDEOH264READBIT pReadBit)
{
    uint8_t cRet = (pReadBit->cData << pReadBit->iBit) & 0x00000080;

    pReadBit->iBit++;
    if (8 == pReadBit->iBit)
    {
        pReadBit->iBit = 0;
        pReadBit->iByte++;
        pReadBit->cData <<= 8;
        pReadBit->cData += pReadBit->pData[pReadBit->iByte];
        if ((uint32_t)0x00000003 == (pReadBit->cData & (uint32_t)0x00FFFFFF))
        {
            /* 根据标准 7.4.1 对 emulation_prevention_three_byte 的描述
             * NALU 出现以下字节序列时，需要在最后两个字节间插入一个防竞争字节 0x03
             * 0x00 0x00 0x00
             * 0x00 0x00 0x01
             * 0x00 0x00 0x02
             * 0x00 0x00 0x03
             * 插入的防竞争字节仅占位，无含义，解码器应当丢弃该字节
             */
            if (((pReadBit->iByte + 1) < pReadBit->nDataLen) && (pReadBit->pData[pReadBit->iByte + 1] <= 3))
            {
                pReadBit->cData &= 0xFFFFFF00;
                pReadBit->iByte++;
                pReadBit->cData += pReadBit->pData[pReadBit->iByte];
            }
        }
    }
    if (0 != cRet) { cRet = 1; }

    return cRet;
}

uint32_t H264ReadBits(PVIDEOH264READBIT pReadBit, uint8_t nLen)
{
    pReadBit->cValue.u32 = 0;
    if (nLen <= 32)
    {
        for (uint8_t iLoop = 0; iLoop < nLen; iLoop++)
        {
            pReadBit->cValue.u32 <<= 1;
            pReadBit->cValue.u32 |= H264ReadBit(pReadBit);
        }
    }

    return pReadBit->cValue.u32;
}
