#include <stdio.h>
#include <string.h>

#include "HAN_FileConversion.h"

#define MSG_FILE_PRO_DIV            ((size_t)100)

#define CONVERT_TRY_MSG_CNT         (100)
#define CONVERT_MIN_MSG_CNT         (CONVERT_TRY_MSG_CNT * 0.9)

static bool GetProtocolId(PFILECONVERSIONPARAM pParam, size_t* pProtocolId);
static void DoMsgInfo(PFILECONVERSIONPARAM pParam, size_t nProtocolId, size_t* pMsgLen, size_t* pMsgCnt);

void ConvertFile(PFILECONVERSIONPARAM pParam)
{
    size_t nMsgLen = 0;
    size_t nMsgCnt = 0;
    FCCHAR pConvertInfo[256];
    size_t nMsgTabIndex = 0;
    bool bDataValid = false;

    if (NULL == pParam->pFilter) { bDataValid = GetProtocolId(pParam, &nMsgTabIndex); }
    else { nMsgTabIndex = *(pParam->pFilter); bDataValid = true; }

    if (true == bDataValid)
    {
        DoMsgInfo(pParam, nMsgTabIndex, &nMsgLen, &nMsgCnt);
        pParam->UpdateProgress(MSG_FILE_PRO_DIV, MSG_FILE_PRO_DIV, pParam->pUserParam);
        Fc_snprintf(pConvertInfo, ArrLen(pConvertInfo), TEXT(FILE_CONVERSION_PRINT_OK_TEXT), pParam->nSrcDataSize, nMsgLen, nMsgCnt);
        pParam->PrintReport(pConvertInfo, pParam->pUserParam);
    }
    else
    {
        pParam->PrintReport(TEXT(FILE_CONVERSION_PRINT_NO_DATA), pParam->pUserParam);
    }
}

static bool GetProtocolId(PFILECONVERSIONPARAM pParam, size_t* pProtocolId)
{
    bool bRet = false;
    size_t i = 0;
    int nGetMsgLen;
    size_t nMsgCnt = 0;
    uint32_t nProtocolCnt = pParam->pConvertMsgInfo->nFileConversionCnt;
    uint32_t pValidMsgCnt[HAN_FILE_CONVERSION_INFO_MAX];
    PFILECONVERSION pMsgInfo = pParam->pConvertMsgInfo->pFileConversion;
    uint8_t* pBuf = pParam->pSrcData;
    
    memset(pValidMsgCnt, 0x00, sizeof(*pValidMsgCnt) * nProtocolCnt);

    while (i < pParam->nSrcDataSize)
    {
        for (size_t j = 0; j < nProtocolCnt; j++)
        {
            nGetMsgLen = pMsgInfo[j].ReadMessage(&pBuf[i], (HANINT)(pParam->nSrcDataSize - i), pParam->pUserSetting, NULL);
            if (0 < nGetMsgLen)
            {
                nMsgCnt++;
                i += nGetMsgLen - 1;
                pValidMsgCnt[j]++;
                break;
            }
        }
        if (CONVERT_TRY_MSG_CNT <= nMsgCnt)
        {
            for (size_t j = 0; j < nProtocolCnt; j++)
            {
                if (CONVERT_MIN_MSG_CNT < pValidMsgCnt[j])
                {
                    *pProtocolId = j;
                    bRet = true;
                }
            }
            break;
        }
        i++;
    }

    return bRet;
}
static void DoMsgInfo(PFILECONVERSIONPARAM pParam, size_t nProtocolId, size_t* pMsgLen, size_t* pMsgCnt)
{
    size_t i = 0;
    size_t nCurrentPro = 0;
    size_t nPrevPro = 0;
    int nGetMsgLen = 0;
    PFILECONVERSION pMsgInfo = &(pParam->pConvertMsgInfo->pFileConversion[nProtocolId]);
    uint8_t* pBuf = pParam->pSrcData;

    pParam->WriteFile(pMsgInfo->pTitle, (HANSIZE)Fc_strlen(pMsgInfo->pTitle), pParam->pDestFile);
    pParam->PrintReport(pMsgInfo->pMsgName, pParam->pUserParam);

    while (i < pParam->nSrcDataSize)
    {
        /* 更新进度条 */
        HANDOUBLE nProDiv = (HANDOUBLE)pParam->nSrcDataSize / (HANDOUBLE)MSG_FILE_PRO_DIV;
        nCurrentPro = (size_t)((HANDOUBLE)i / nProDiv);

        if (nCurrentPro != nPrevPro) { pParam->UpdateProgress((uint32_t)nCurrentPro, MSG_FILE_PRO_DIV, pParam->pUserParam); }
        /* 尝试解析 */
        nGetMsgLen = pMsgInfo->ReadMessage(&pBuf[i], (HANINT)(pParam->nSrcDataSize - i), pParam->pUserSetting, pParam->pDestFile);
        if (0 < nGetMsgLen)
        {
            (*pMsgLen) += nGetMsgLen;
            i += nGetMsgLen - 1;
            (*pMsgCnt)++;
        }
        nPrevPro = nCurrentPro;
        i++;
    }
}
