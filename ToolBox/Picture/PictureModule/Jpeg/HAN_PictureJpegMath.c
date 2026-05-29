#define _USE_MATH_DEFINES
#include <math.h>

#include "HAN_PictureJpegMath.h"

#if 1 /* 慢速算法（标准数学公式），这个算法真的很慢 */
static const PICTUREJPEGFLOATMATRIX sg_pCosTable = {
    { 1,  0.980785280403230,  0.923879532511287,  0.831469612302545,  0.707106781186548,  0.555570233019602,  0.382683432365090,  0.195090322016128, },
    { 1,  0.831469612302545,  0.382683432365090, -0.195090322016128, -0.707106781186547, -0.980785280403230, -0.923879532511287, -0.555570233019602, },
    { 1,  0.555570233019602, -0.382683432365090, -0.980785280403230, -0.707106781186548,  0.195090322016128,  0.923879532511287,  0.831469612302545, },
    { 1,  0.195090322016128, -0.923879532511287, -0.555570233019602,  0.707106781186547,  0.831469612302545, -0.382683432365090, -0.980785280403231, },
    { 1, -0.195090322016128, -0.923879532511287,  0.555570233019602,  0.707106781186548, -0.831469612302545, -0.382683432365091,  0.980785280403230, },
    { 1, -0.555570233019602, -0.382683432365090,  0.980785280403230, -0.707106781186547, -0.195090322016128,  0.923879532511287, -0.831469612302545, },
    { 1, -0.831469612302545,  0.382683432365090,  0.195090322016129, -0.707106781186547,  0.980785280403231, -0.923879532511286,  0.555570233019602, },
    { 1, -0.980785280403230,  0.923879532511287, -0.831469612302545,  0.707106781186547, -0.555570233019602,  0.382683432365090, -0.195090322016129, },
};
static const PICTUREJPEGFLOATMATRIX sg_pCxTable = {
    {       0.5, M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
    { M_SQRT1_2,         1,         1,         1,         1,         1,         1,         1, },
};

void PictureJpegDCT(PICTUREJPEGDCTMATRIX2D* pDest, const PICTUREJPEGDCTMATRIX2D* pSrc, HANSIZE nLen)
{
    PICTUREJPEGDCTMATRIX2D pTemp;
    for (HANSIZE nId = 0; nId < nLen; nId++)
    {
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
            {
                pTemp[iLoop][jLoop] = pSrc[nId][iLoop][jLoop] - 128;
            }
        }
        for (HANSIZE uLoop = 0; uLoop < 8; uLoop++)
        {
            for (HANSIZE vLoop = 0; vLoop < 8; vLoop++)
            {
                pDest[nId][uLoop][vLoop] = 0;
                for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
                {
                    for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
                    {
                        if (0 != pTemp[iLoop][jLoop])
                        {
                            pDest[nId][uLoop][vLoop] += (PICTUREJPEGNUMBER)(pTemp[iLoop][jLoop] * sg_pCosTable[iLoop][uLoop] * sg_pCosTable[jLoop][vLoop]);
                        }
                    }
                }
                pDest[nId][uLoop][vLoop] = (PICTUREJPEGNUMBER)(pDest[nId][uLoop][vLoop] * sg_pCxTable[uLoop][vLoop] / 4);
            }
        }
    }
}
void PictureJpegIDCT(PICTUREJPEGDCTMATRIX2D* pDest, const PICTUREJPEGDCTMATRIX2D* pSrc, HANSIZE nLen)
{
    for (HANSIZE nId = 0; nId < nLen; nId++)
    {
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
            {
                pDest[nId][iLoop][jLoop] = 0;
                for (HANSIZE uLoop = 0; uLoop < 8; uLoop++)
                {
                    for (HANSIZE vLoop = 0; vLoop < 8; vLoop++)
                    {
                        if (0 != pSrc[nId][uLoop][vLoop])
                        {
                            pDest[nId][iLoop][jLoop] += (PICTUREJPEGNUMBER)(pSrc[nId][uLoop][vLoop] * sg_pCxTable[uLoop][vLoop] * sg_pCosTable[iLoop][uLoop] * sg_pCosTable[jLoop][vLoop]);
                        }
                    }
                }
                pDest[nId][iLoop][jLoop] = pDest[nId][iLoop][jLoop] / 4 + 128;
            }
        }
    }
}
#endif

#if 1 /* 快速算法（行列分开计算），耗时不到慢速算法的 1/3 */
/* 这里的系数已经简化到一维变换的每一项都是单个输入 × 单个浮点常数，测试下来与整数乘法再移位在耗时上没有多少区别，所以保留了高精度的浮点运算 */
static const PICTUREJPEGFLOATMATRIX sg_pFastCoefTable = {
    { 0.353553390593274,  0.490392640201615,  0.461939766255643,  0.415734806151273,  0.353553390593274,  0.277785116509801,  0.191341716182545,  0.097545161008064, },
    { 0.353553390593274,  0.415734806151273,  0.191341716182545, -0.097545161008064, -0.353553390593274, -0.490392640201615, -0.461939766255643, -0.277785116509801, },
    { 0.353553390593274,  0.277785116509801, -0.191341716182545, -0.490392640201615, -0.353553390593274,  0.097545161008064,  0.461939766255643,  0.415734806151273, },
    { 0.353553390593274,  0.097545161008064, -0.461939766255643, -0.277785116509801,  0.353553390593274,  0.415734806151273, -0.191341716182545, -0.490392640201615, },
    { 0.353553390593274, -0.097545161008064, -0.461939766255643,  0.277785116509801,  0.353553390593274, -0.415734806151273, -0.191341716182546,  0.490392640201615, },
    { 0.353553390593274, -0.277785116509801, -0.191341716182545,  0.490392640201615, -0.353553390593274, -0.097545161008064,  0.461939766255643, -0.415734806151273, },
    { 0.353553390593274, -0.415734806151273,  0.191341716182545,  0.097545161008064, -0.353553390593274,  0.490392640201615, -0.461939766255643,  0.277785116509801, },
    { 0.353553390593274, -0.490392640201615,  0.461939766255643, -0.415734806151273,  0.353553390593274, -0.277785116509801,  0.191341716182545, -0.097545161008064, },
};

static inline void FastIDCT1DTrans(PICTUREJPEGDCTMATRIX2D pDest, const PICTUREJPEGDCTROW pSrc, HANSIZE nRow);
static inline void FastIDCT1D(PICTUREJPEGDCTROW pDest, const PICTUREJPEGDCTROW pSrc);

void PictureJpegFastIDCT(PICTUREJPEGDCTMATRIX2D* pDest, const PICTUREJPEGDCTMATRIX2D* pSrc, HANSIZE nLen)
{
    PICTUREJPEGDCTMATRIX2D pTmp;
    PICTUREJPEGDCTROW pColDest;
    
    for (HANSIZE iLoop = 0; iLoop < nLen; iLoop++)
    {
        /* 对每一行做 1D IDCT，并直接转置 */
        for (HANSIZE nRow = 0; nRow < 8; nRow++)
        {
            FastIDCT1DTrans(pTmp, pSrc[iLoop][nRow], nRow);
        }
        
        /* 对每一列做 1D IDCT */
        for (HANSIZE nCol = 0; nCol < 8; nCol++)
        {
            FastIDCT1D(pColDest, pTmp[nCol]);
            /* 写回结果 */
            for (HANSIZE nRow = 0; nRow < 8; nRow++)
            {
                pDest[iLoop][nRow][nCol] = pColDest[nRow] + 128;
            }
        }
    }
}
/* 一维 IDCT */
static inline void FastIDCT1DTrans(PICTUREJPEGDCTMATRIX2D pDest, const PICTUREJPEGDCTROW pSrc, HANSIZE nRow)
{
    if (
        (0 == pSrc[1]) &&
        (0 == pSrc[2]) &&
        (0 == pSrc[3]) &&
        (0 == pSrc[4]) &&
        (0 == pSrc[5]) &&
        (0 == pSrc[6]) &&
        (0 == pSrc[7])
    )
    {
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            /* 第 x 行放在第 x 列 */
            pDest[iLoop][nRow] = (PICTUREJPEGNUMBER)(pSrc[0] * sg_pFastCoefTable[iLoop][0]);
        }
    }
    else
    {
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            pDest[iLoop][nRow] = 0;
            for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
            {
                if (0 != pSrc[jLoop]) { pDest[iLoop][nRow] += (PICTUREJPEGNUMBER)(pSrc[jLoop] * sg_pFastCoefTable[iLoop][jLoop]); }
            }
        }
    }
}
static inline void FastIDCT1D(PICTUREJPEGDCTROW pDest, const PICTUREJPEGDCTROW pSrc)
{
    if (
        (0 == pSrc[1]) &&
        (0 == pSrc[2]) &&
        (0 == pSrc[3]) &&
        (0 == pSrc[4]) &&
        (0 == pSrc[5]) &&
        (0 == pSrc[6]) &&
        (0 == pSrc[7])
    )
    {
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            pDest[iLoop] = (PICTUREJPEGNUMBER)(pSrc[0] * sg_pFastCoefTable[iLoop][0]);
        }
    }
    else
    {
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            pDest[iLoop] = 0;
            for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
            {
                if (0 != pSrc[jLoop]) { pDest[iLoop] += (PICTUREJPEGNUMBER)(pSrc[jLoop] * sg_pFastCoefTable[iLoop][jLoop]); }
            }
        }
    }
}
#endif
