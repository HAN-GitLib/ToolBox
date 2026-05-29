#include "HAN_PictureJpegIPTC.h"

static BOOL PictureJpegDecodeIPTCEnvelopeRecord(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC);
static BOOL PictureJpegDecodeIPTCApplicationRecord(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC);

BOOL PictureJpegDecodeIPTC(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    BOOL bRet = TRUE;

    switch (pDecodeIPTC->pIPTCData->nRecordId) {
        case 1: { bRet = PictureJpegDecodeIPTCEnvelopeRecord(pDecodeIPTC); } break;
        case 2: { bRet = PictureJpegDecodeIPTCApplicationRecord(pDecodeIPTC); } break;

        default: {
            printf("unknown IPTC Record ID: %u\n", pDecodeIPTC->pIPTCData->nRecordId);
            bRet = FALSE;
        } break;
    }

    return bRet;
}

HANSIZE PictureJpegReadIPTC(const uint8_t* pData, HANSIZE nLen, uint8_t nTagMarker, PPICTUREJPEGIPTCDATA pIPTCData)
{
    HANSIZE nRet = 0;
    HANSIZE nOffset = 0;
    HANSIZE nDataLen;
    HANSIZE nDataLenLen;

    while (nOffset < nLen)
    {
        if (nTagMarker == pData[nOffset])
        {
            nOffset = 3;
            nDataLen = ReadJpegData2ByteMSB(&pData[nOffset]);
            nOffset = 5;
            if (0x7FFF < nDataLen)
            {
                nDataLenLen = nDataLen & 0x7FFF;
                nDataLen = 0;
                for (HANSIZE iLoop = 0; iLoop < nDataLenLen; iLoop++)
                {
                    nDataLen <<= 8;
                    nDataLen += pData[nOffset];
                    nOffset++;
                }
            }
            if ((nOffset + nDataLen) <= nLen)
            {
                pIPTCData->nRecordId = pData[1];
                pIPTCData->nDataSetId = pData[2];
                pIPTCData->nLen = nDataLen;
                pIPTCData->pData = &pData[nOffset];
                nRet = nOffset + nDataLen;
                break;
            }
        }
        nOffset++;
    }

    return nRet;
}

#if 1 // Envelope Record
static void PictureJpegDecodeIPTCEnvelopeRecord90(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    const uint8_t* pData = pDecodeIPTC->pIPTCData->pData;

    HAN_strncpy(pDecodeIPTC->pName, TEXT("字符编码"), pDecodeIPTC->nNameSize);
    if (!memcmp(pData, "\033%G", 3))
    {
        HAN_strncpy(pDecodeIPTC->pValue, TEXT("UTF-8"), pDecodeIPTC->nValueSize);
        pDecodeIPTC->cCodePage = CP_UTF8;
    }
    else
    {
        HAN_strncpy(pDecodeIPTC->pValue, TEXT("其它"), pDecodeIPTC->nValueSize);
        pDecodeIPTC->cCodePage = CP_ACP;
    }
}
static BOOL PictureJpegDecodeIPTCEnvelopeRecord(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    BOOL bRet = TRUE;

    switch (pDecodeIPTC->pIPTCData->nDataSetId) {
        case 90: {
            PictureJpegDecodeIPTCEnvelopeRecord90(pDecodeIPTC);
        } break;

        default: {
            printf("unknown IPTC Envelope DataSet ID: %u, len: " HANSIZE_PRINT_FORMAT "\n",
                pDecodeIPTC->pIPTCData->nDataSetId,
                pDecodeIPTC->pIPTCData->nLen
            );
            bRet = FALSE;
        } break;
    }

    return bRet;
}
#endif
#if 1 // Application Record
static void PictureJpegDecodeIPTCApplicationRecord0(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    HAN_snprintf(pDecodeIPTC->pName, pDecodeIPTC->nNameSize, TEXT("版本号"));
    HAN_snprintf(pDecodeIPTC->pValue, pDecodeIPTC->nValueSize, TEXT("%u"), ReadJpegData2ByteMSB(pDecodeIPTC->pIPTCData->pData));
}
static void PictureJpegDecodeIPTCApplicationRecord55(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    const uint8_t* pData = pDecodeIPTC->pIPTCData->pData;
    CHAR pText[3][5];
    uint16_t pNum[3];
    memcpy(pText[0], &pData[0], 4); pText[0][4] = '\0';
    memcpy(pText[1], &pData[4], 2); pText[1][2] = '\0';
    memcpy(pText[2], &pData[6], 2); pText[2][2] = '\0';
    pNum[0] = (uint16_t)strtoul(pText[0], NULL, 10);
    pNum[1] = (uint16_t)strtoul(pText[1], NULL, 10);
    pNum[2] = (uint16_t)strtoul(pText[2], NULL, 10);
    HAN_snprintf(pDecodeIPTC->pName, pDecodeIPTC->nNameSize, TEXT("创建日期"));
    HAN_snprintf(pDecodeIPTC->pValue, pDecodeIPTC->nValueSize, TEXT("%u.%u.%u"), pNum[0], pNum[1], pNum[2]);
}
static void PictureJpegDecodeIPTCApplicationRecord60(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    const uint8_t* pData = pDecodeIPTC->pIPTCData->pData;
    CHAR pText[32];
    pText[0] = pData[0];
    pText[1] = pData[1];
    pText[2] = ':';
    pText[3] = pData[2];
    pText[4] = pData[3];
    pText[5] = ':';
    pText[6] = pData[4];
    pText[7] = pData[5];
    pText[8] = '\0';
    if (6 < pDecodeIPTC->pIPTCData->nLen)
    {
        pText[8] = pData[6];
        pText[9] = pData[7];
        pText[10] = pData[8];
        pText[11] = ':';
        pText[12] = pData[9];
        pText[13] = pData[10];
        pText[14] = '\0';
    }
    HAN_snprintf(pDecodeIPTC->pName, pDecodeIPTC->nNameSize, TEXT("创建时间"));
    HAN_snprintf(pDecodeIPTC->pValue, pDecodeIPTC->nValueSize, HANPSTR_PRINT_PCHAR_FORMAT, pText);
}
static void PictureJpegDecodeIPTCApplicationRecord120(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    CHAR pData[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    WCHAR pBuf[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANINT nStrSize = (HANINT)pDecodeIPTC->nValueSize;
    HANINT nBufSize = HAN_PICTURE_JPEG_TEXT_BUF_SIZE;

    memcpy(pData, pDecodeIPTC->pIPTCData->pData, pDecodeIPTC->pIPTCData->nLen);
    pData[pDecodeIPTC->pIPTCData->nLen] = '\0';
    HAN_snprintf(pDecodeIPTC->pName, pDecodeIPTC->nNameSize, TEXT("说明/摘要"));
    MultiByteToHANStr(
        pDecodeIPTC->cCodePage, 0,
        pData, -1,
        pDecodeIPTC->pValue, &nStrSize,
        pBuf, &nBufSize,
        NULL, NULL
    );
}
static BOOL PictureJpegDecodeIPTCApplicationRecord(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC)
{
    BOOL bRet = TRUE;

    switch (pDecodeIPTC->pIPTCData->nDataSetId) {
        case 0: { PictureJpegDecodeIPTCApplicationRecord0(pDecodeIPTC); } break;
        case 55: { PictureJpegDecodeIPTCApplicationRecord55(pDecodeIPTC); } break;
        case 60: { PictureJpegDecodeIPTCApplicationRecord60(pDecodeIPTC); } break;
        case 120: { PictureJpegDecodeIPTCApplicationRecord120(pDecodeIPTC); } break;

        default: {
            printf("unknown IPTC Application DataSet ID: %u, len: " HANSIZE_PRINT_FORMAT "\n",
                pDecodeIPTC->pIPTCData->nDataSetId,
                pDecodeIPTC->pIPTCData->nLen
            );
            bRet = FALSE;
        } break;
    }

    return bRet;
}
#endif
