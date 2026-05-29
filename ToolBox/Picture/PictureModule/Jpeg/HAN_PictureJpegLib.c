#include "HAN_PictureJpegLib.h"

void PictureJpegPrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nLen)
{
    HANSIZE nPrintLen = nLen;
    HANSIZE nOffset;

    if (10 < nPrintLen) { nPrintLen = 10; }
    nOffset = 0;
    for (HANSIZE iLoop = 0; iLoop < nPrintLen; iLoop++)
    {
        HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("%02X "), pData[iLoop]);
        nOffset += 3;
    }
    if (10 < nLen) { HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("...")); }
}
