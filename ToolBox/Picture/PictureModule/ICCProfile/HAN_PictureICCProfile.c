#include <Windows.h>

#include "HAN_PictureICCProfile.h"

static inline uint16_t ICCProfileRead2Bytes(const uint8_t pData[2]);
static inline uint32_t ICCProfileRead4Bytes(const uint8_t pData[4]);

size_t DecodeICCProfileHeader(const uint8_t* pData, size_t nLen, PICCPROFILEHEADER pHeader)
{
    size_t nRet = 0;
    uint32_t nFileSize = ICCProfileRead4Bytes(pData);
    uint32_t nTagNum;
    uint32_t nOffset;

    if (nFileSize <= nLen)
    {
        pHeader->nFileSize = nFileSize; nOffset = 4;
        memcpy(pHeader->pCMMType, &pData[nOffset], 4); nOffset += 4;
        pHeader->nVersion = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        memcpy(pHeader->pClass, &pData[nOffset], 4); nOffset += 4;
        memcpy(pHeader->pColorSpace, &pData[nOffset], 4); nOffset += 4;
        memcpy(pHeader->pPCS, &pData[nOffset], 4); nOffset += 4;
        pHeader->dateTime.nYear = ICCProfileRead2Bytes(&pData[nOffset]); nOffset += 2;
        pHeader->dateTime.nMonth = ICCProfileRead2Bytes(&pData[nOffset]); nOffset += 2;
        pHeader->dateTime.nDay = ICCProfileRead2Bytes(&pData[nOffset]); nOffset += 2;
        pHeader->dateTime.nHour = ICCProfileRead2Bytes(&pData[nOffset]); nOffset += 2;
        pHeader->dateTime.nMinute = ICCProfileRead2Bytes(&pData[nOffset]); nOffset += 2;
        pHeader->dateTime.nSecond = ICCProfileRead2Bytes(&pData[nOffset]); nOffset += 2;
        memcpy(pHeader->pSignature, &pData[nOffset], 4); nOffset += 4;
        memcpy(pHeader->pPrimaryPlatform, &pData[nOffset], 4); nOffset += 4;
        pHeader->cFlags = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        memcpy(pHeader->pDeviceManufacturer, &pData[nOffset], 4); nOffset += 4;
        memcpy(pHeader->pDeviceModel, &pData[nOffset], 4); nOffset += 4;
        pHeader->pDeviceAttributes[0] = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        pHeader->pDeviceAttributes[1] = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        pHeader->cRenderingIntent = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        pHeader->pPCSIlluminant[0] = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        pHeader->pPCSIlluminant[1] = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        pHeader->pPCSIlluminant[2] = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        memcpy(pHeader->pCreator, &pData[nOffset], 4); nOffset += 4;

        nTagNum = ICCProfileRead4Bytes(&pData[128]);
        nRet = sizeof(ICCPROFILEDATA) + (sizeof(ICCPROFILETAG) * nTagNum);
    }

    return nRet;
}

void DecodeICCProfileData(const uint8_t* pData, size_t nLen, PICCPROFILEDATA pICCData)
{
    uint32_t nTagNum;
    uint32_t nOffset = 128;

    nTagNum = ICCProfileRead4Bytes(&pData[nOffset]);
    if ((128 + 4 + (nTagNum * 12)) <= nLen)
    {
        pICCData->nTagNum = nTagNum;
        nOffset += 4;
        for (uint32_t iLoop = 0; iLoop < nTagNum; iLoop++)
        {
            memcpy(pICCData->pTag[iLoop].pTagId, &pData[nOffset], 4); nOffset += 4;
            pICCData->pTag[iLoop].pData = &pData[ICCProfileRead4Bytes(&pData[nOffset])]; nOffset += 4;
            pICCData->pTag[iLoop].nLen = ICCProfileRead4Bytes(&pData[nOffset]); nOffset += 4;
        }
    }
}

static inline uint16_t ICCProfileRead2Bytes(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline uint32_t ICCProfileRead4Bytes(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}
