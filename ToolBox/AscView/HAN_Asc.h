#ifndef HAN_ASC_H
#define HAN_ASC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_Lib\HAN_windows.h"
#include "HAN_CANLog.h"

#define HAN_ASC_VIEW_CLASS      TEXT("HAN_AscView")

#define ASC_CAN_LOG_ALLOC_STEP      256
#define ASC_CAN_ID_TEXT_BUF_SIZE    16
#define ASC_CAN_CH_TEXT_BUF_SIZE    16
#define ASC_CSV_LINE_BUF_SIZE       1024

typedef struct tagASCFILE {
    PCANLOG     pCanLog;
    uint32_t    nCanLogCnt;
} ASCFILE, * PASCFILE;
typedef const ASCFILE* PCASCFILE;

void RegisterHANAscView(HINSTANCE hInst);

HANERROR HANOpenAscFile(PASCFILE pAscFile, HANPCSTR pFileName, HANDLE hHeap);
HANERROR AscViewAscToCsv(PASCFILE pAscFile, HANPCSTR pFileName, const uint32_t* pIdFilter, const uint32_t* pChFilter);

#ifdef __cplusplus
}
#endif

#endif
