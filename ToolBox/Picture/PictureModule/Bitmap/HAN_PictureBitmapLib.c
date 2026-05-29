#include "HAN_PictureBitMapLib.h"

typedef void (*COPYPIXELMODE)(PPICTURERGBA pDest, PCPICTURERGBA pSrc);

static HANSIZE GetBitmapRowSize(uint32_t nWidth, WORD nBitCnt);

static void DecodeBitmap1Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback);
static void DecodeBitmap4Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback);
static void DecodeBitmap8Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback);
static void DecodeBitmap16Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback);
static void DecodeBitmap16BitStd(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, COPYPIXELMODE CopyCallback);
static void DecodeBitmap16BitMask(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, COPYPIXELMODE CopyCallback);
static void DecodeBitmap24Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback);
static void DecodeBitmap32Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback);

static void CopyRGBA(PPICTURERGBA pDest, PCPICTURERGBA pSrc);
static void CopyMask(PPICTURERGBA pDest, PCPICTURERGBA pSrc);

void GetBitmapPixelSizeName(WORD nSize, HANPSTR pText, HANSIZE nTextSize)
{
    switch (nSize) {
        case 1: { HAN_strncpy(pText, TEXT("黑白位图"), nTextSize); } break;
        case 4: { HAN_strncpy(pText, TEXT("16色位图"), nTextSize); } break;
        case 8: { HAN_strncpy(pText, TEXT("256色位图"), nTextSize); } break;
        case 16: { HAN_strncpy(pText, TEXT("高彩色"), nTextSize); } break;
        case 24: { HAN_strncpy(pText, TEXT("真彩色"), nTextSize); } break;
        case 32: { HAN_strncpy(pText, TEXT("带Alpha通道的真彩色"), nTextSize); } break;
        default: { HAN_strncpy(pText, TEXT("无效值"), nTextSize); }
    }
}

void CopyBitmapToPictureBitmapInfo(PPICTUREBITMAPINFO pInfo, const BITMAP* pBitmap)
{
    pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pInfo->bmiHeader.biWidth = pBitmap->bmWidth;
    if (0 < pBitmap->bmHeight) { pInfo->bmiHeader.biHeight = pBitmap->bmHeight; }
    else { pInfo->bmiHeader.biHeight = -pBitmap->bmHeight; }
    pInfo->bmiHeader.biPlanes = pBitmap->bmPlanes;
    pInfo->bmiHeader.biBitCount = pBitmap->bmBitsPixel;
    pInfo->bmiHeader.biSizeImage = pBitmap->bmWidthBytes * pBitmap->bmHeight;
    pInfo->bmiHeader.biCompression = BI_RGB;
}

BOOL DecodeBitmapColor(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha)
{
    BOOL bRet = TRUE;
    PICTURERESOLUTION pxResolution = {
        .pxWidth = pInfo->bmiHeader.biWidth,
        .pxHeight = pInfo->bmiHeader.biHeight,
    };

    UpdatePictureMap(pPicture, &pxResolution);
    switch (pInfo->bmiHeader.biBitCount) {
        case 1: { DecodeBitmap1Bit(pPicture, pInfo, pAlpha, CopyRGBA); } break;
        case 4: { DecodeBitmap4Bit(pPicture, pInfo, pAlpha, CopyRGBA); } break;
        case 8: { DecodeBitmap8Bit(pPicture, pInfo, pAlpha, CopyRGBA); } break;
        case 16: { DecodeBitmap16Bit(pPicture, pInfo, pAlpha, CopyRGBA); } break;
        case 24: { DecodeBitmap24Bit(pPicture, pInfo, pAlpha, CopyRGBA); } break;
        case 32: { DecodeBitmap32Bit(pPicture, pInfo, pAlpha, CopyRGBA); } break;

        default: { bRet = FALSE; } break;
    }

    return bRet;
}
BOOL DecodeBitmapMask(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo)
{
    BOOL bRet = TRUE;
    BOOL bAlpha;
    PICTURERESOLUTION pxResolution = {
        .pxWidth = pInfo->bmiHeader.biWidth,
        .pxHeight = pInfo->bmiHeader.biHeight,
    };

    UpdatePictureMap(pPicture, &pxResolution);
    switch (pInfo->bmiHeader.biBitCount) {
        case 1: { DecodeBitmap1Bit(pPicture, pInfo, &bAlpha, CopyMask); } break;
        case 4: { DecodeBitmap4Bit(pPicture, pInfo, &bAlpha, CopyMask); } break;
        case 8: { DecodeBitmap8Bit(pPicture, pInfo, &bAlpha, CopyMask); } break;
        case 16: { DecodeBitmap16Bit(pPicture, pInfo, &bAlpha, CopyMask); } break;
        case 24: { DecodeBitmap24Bit(pPicture, pInfo, &bAlpha, CopyMask); } break;
        case 32: { DecodeBitmap32Bit(pPicture, pInfo, &bAlpha, CopyMask); } break;

        default: { bRet = FALSE; } break;
    }

    return bRet;
}

static HANSIZE GetBitmapRowSize(uint32_t nWidth, WORD nBitCnt)
{
    return ((nWidth * nBitCnt + 31) / 32) * 4;
}

static void DecodeBitmap1Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    PCPICTURERGBA pColors = pInfo->colorTable.pColors;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nRowOffset = 0;
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;
    uint8_t nQuadId;
    uint8_t nI = 0;

    *pAlpha = FALSE;

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = nRowOffset;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            nQuadId = (pData[nOffset] >> (7 - nI)) & 0x01;
            rgbaPixel = pColors[nQuadId];
            rgbaPixel.a = 0xFF;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);

            nI = (nI + 1) % 8;
            if (0 == nI) { nOffset++; }
        }
        nRowOffset += nRowSize;
    }
}
static void DecodeBitmap4Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    PCPICTURERGBA pColors = pInfo->colorTable.pColors;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nRowOffset = 0;
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;
    uint8_t nQuadId;
    uint8_t nI = 0;

    *pAlpha = FALSE;

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = nRowOffset;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            if (0 == nI) { nQuadId = pData[nOffset] >> 4; }
            else { nQuadId = pData[nOffset] & 0x0F; }
            rgbaPixel = pColors[nQuadId];
            rgbaPixel.a = 0xFF;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);

            nI = 1 - nI;
            if (0 == nI) { nOffset++; }
        }
        nRowOffset += nRowSize;
    }
}
static void DecodeBitmap8Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    PCPICTURERGBA pColors = pInfo->colorTable.pColors;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nRowOffset = 0;
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;
    uint8_t nQuadId;

    *pAlpha = FALSE;

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = nRowOffset;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            nQuadId = pData[nOffset];
            rgbaPixel = pColors[nQuadId]; nOffset++;
            rgbaPixel.a = 0xFF;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);
        }
        nRowOffset += nRowSize;
    }
}
static void DecodeBitmap16Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback)
{
    *pAlpha = FALSE;
    switch (pInfo->bmiHeader.biCompression) {
        case BI_RGB: { DecodeBitmap16BitStd(pPicture, pInfo, CopyCallback); } break;
        case BI_BITFIELDS: { DecodeBitmap16BitMask(pPicture, pInfo, CopyCallback); } break;
        default: { } break;
    }
}
static void DecodeBitmap16BitStd(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    const uint16_t* pPixel;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = 0;
        pPixel = (const uint16_t*)pData;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            rgbaPixel.r = (pPixel[nOffset] >> 10) & 0x1F;
            rgbaPixel.g = (pPixel[nOffset] >> 5) & 0x1F;
            rgbaPixel.b = pPixel[nOffset] & 0x1F;
            nOffset++;
            rgbaPixel.b = rgbaPixel.b * 255 / 0x1F;
            rgbaPixel.g = rgbaPixel.g * 255 / 0x1F;
            rgbaPixel.r = rgbaPixel.r * 255 / 0x1F;
            rgbaPixel.a = 0xFF;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);
        }
        pData = &pData[nRowSize];
    }
}
static void DecodeBitmap16BitMask(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    const uint32_t* pMask = pInfo->colorTable.pBitFileds;
    const uint16_t* pPixel;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;
    uint8_t pMoveLen[3];
    uint8_t pSize[3];

    for (uint8_t iLoop = 0; iLoop < 3; iLoop++)
    {
        pMoveLen[iLoop] = 0;
        while (0 == ((pMask[iLoop] >> pMoveLen[iLoop]) & 0x00000001)) { pMoveLen[iLoop]++; }
        pSize[iLoop] = pMask[iLoop] >> pMoveLen[iLoop];
    }

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = 0;
        pPixel = (const uint16_t*)pData;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            rgbaPixel.r = (pPixel[nOffset] & pMask[0]) >> pMoveLen[0];
            rgbaPixel.g = (pPixel[nOffset] & pMask[1]) >> pMoveLen[1];
            rgbaPixel.b = (pPixel[nOffset] & pMask[2]) >> pMoveLen[2];
            nOffset++;
            rgbaPixel.b = rgbaPixel.b * 255 / pSize[0];
            rgbaPixel.g = rgbaPixel.g * 255 / pSize[1];
            rgbaPixel.r = rgbaPixel.r * 255 / pSize[2];
            rgbaPixel.a = 0xFF;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);
        }
        pData = &pData[nRowSize];
    }
}
static void DecodeBitmap24Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    PCPICTURERGB pRGB;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;

    *pAlpha = FALSE;

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = 0;
        pRGB = (PCPICTURERGB)pData;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            rgbaPixel.b = pRGB[nOffset].b;
            rgbaPixel.g = pRGB[nOffset].g;
            rgbaPixel.r = pRGB[nOffset].r;
            nOffset++;
            rgbaPixel.a = 0xFF;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);
        }
        pData = &pData[nRowSize];
    }
}
static void DecodeBitmap32Bit(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha, COPYPIXELMODE CopyCallback)
{
    const uint8_t* pData = pInfo->pixelData.pData;
    PCPICTURERGBA pRGBA;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    uint32_t pxWidth = pInfo->bmiHeader.biWidth;
    uint32_t pxHeight = pInfo->bmiHeader.biHeight;
    HANSIZE nRowSize = GetBitmapRowSize(pxWidth, pInfo->bmiHeader.biBitCount);
    HANSIZE nOffset;
    PICTURERGBA rgbaPixel;

    *pAlpha = FALSE;

    for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
    {
        nOffset = 0;
        pRGBA = (PCPICTURERGBA)pData;
        for (uint32_t nCol = 0; nCol < pxWidth; nCol++)
        {
            rgbaPixel = pRGBA[nOffset];
            nOffset++;
            CopyCallback(&pPictureMap[nRow][nCol], &rgbaPixel);
            if ((0x00 < rgbaPixel.a) && (rgbaPixel.a < 0xFF)) { *pAlpha = TRUE; }
        }
        pData = &pData[nRowSize];
    }
}

static void CopyRGBA(PPICTURERGBA pDest, PCPICTURERGBA pSrc)
{
    *pDest = *pSrc;
}
static void CopyMask(PPICTURERGBA pDest, PCPICTURERGBA pSrc)
{
    pDest->a = 255 - (uint8_t)(((uint32_t)299 * (uint32_t)pSrc->r + (uint32_t)587 * (uint32_t)pSrc->g + (uint32_t)114 * (uint32_t)pSrc->b) / (uint32_t)1000);
}
