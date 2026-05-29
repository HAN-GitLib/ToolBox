#include "HAN_PictureLib.h"

SIZE_T GetPictureInfoMemSize(HANSIZE nCnt)
{
    return (sizeof(HANPICTUREINFO) + (nCnt * sizeof(HANPPICTURE)));
}

SIZE_T GetPictureMemSize(PPICTURERESOLUTION pResolution)
{
    uint32_t nRowSize = pResolution->pxWidth * sizeof(PICTURERGBA);
    uint32_t nPictureSize = pResolution->pxHeight * nRowSize;
    return (sizeof(HANPICTURE) + nPictureSize + (sizeof(PPICTURERGBA) * pResolution->pxHeight));
}

void UpdatePictureMap(HANPPICTURE pPicture, PPICTURERESOLUTION pResolution)
{
    HANSIZE nRowSize = pResolution->pxWidth * sizeof(PICTURERGBA);
    HANSIZE nPictureSize = pResolution->pxHeight * nRowSize;
    
    pPicture->pxResolution = *pResolution;
    pPicture->pPictureMap = (PPICTURERGBA*)&(pPicture->pPixel[nPictureSize]);
    for (uint32_t iLoop = 0; iLoop < pResolution->pxHeight; iLoop++)
    {
        pPicture->pPictureMap[iLoop] = (PPICTURERGBA)&(pPicture->pPixel[iLoop * nRowSize]);
    }
}

PICTURERGBA BlendRGBA(PICTURERGBA rgbaFore, PICTURERGBA rgbaBack)
{
    PICTURERGBA rgbaRet;

    rgbaRet.a = (uint32_t)(rgbaFore.a) + (uint32_t)(rgbaBack.a) - ((uint32_t)(rgbaFore.a) * (uint32_t)(rgbaBack.a) / 0xFF);
    rgbaRet.r = ((uint32_t)(rgbaFore.r) * (uint32_t)(rgbaFore.a) + (uint32_t)(rgbaBack.r) * (uint32_t)(rgbaBack.a) * (uint32_t)(0xFF - rgbaFore.a) / 0xFF) / rgbaRet.a;
    rgbaRet.g = ((uint32_t)(rgbaFore.g) * (uint32_t)(rgbaFore.a) + (uint32_t)(rgbaBack.g) * (uint32_t)(rgbaBack.a) * (uint32_t)(0xFF - rgbaFore.a) / 0xFF) / rgbaRet.a;
    rgbaRet.b = ((uint32_t)(rgbaFore.b) * (uint32_t)(rgbaFore.a) + (uint32_t)(rgbaBack.b) * (uint32_t)(rgbaBack.a) * (uint32_t)(0xFF - rgbaFore.a) / 0xFF) / rgbaRet.a;

    return rgbaRet;
}
