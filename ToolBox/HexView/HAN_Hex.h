#ifndef HAN_HEX_H
#define HAN_HEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\ToolBoxTypedef.h"

#define HAN_HEX_VIEW_CLASS              TEXT("HAN_HexView")

#define DEFAULT_START_ADDR              0x00000000
#define DEFAULT_END_ADDR                0x0000FFFF
#define DEFAULT_START_ADDR_TEXT         TEXT("00000000")
#define DEFAULT_END_ADDR_TEXT           TEXT("0000FFFF")

typedef enum {
    INI_HEX_VIEW_START_ADDR,
    INI_HEX_VIEW_END_ADDR,
    INI_HEX_VIEW_CFG_CNT,
} INIHEXVIEWCFGID;

typedef struct tagHEXVIEWCFG {
    HANDATAINI          pSysConfig[INI_HEX_VIEW_CFG_CNT];
} HEXVIEWCFG, * PHEXVIEWCFG;

typedef struct tagHEXRECORD {
    uint8_t     nDataLen;
    uint16_t    cAddr;
    uint8_t     cDataType;
    uint8_t     pData[256];
    uint8_t     cCheckValue;
    BOOL        bCheckOK;
} HEXRECORD, * PHEXRECORD;
typedef const HEXRECORD* PCHEXRECORD;

typedef struct tagHEXFILE {
    char*       pText;
    PHEXRECORD  pHexRecord;
    uint8_t*    pBinData;
    uint32_t    nTextLen;
    uint32_t    nHexRecordCnt;
    ADDRMAP     amBinAddrMap;
} HEXFILE, * PHEXFILE;
typedef const HEXFILE* PCHEXFILE;

void RegisterHANHexView(HINSTANCE hInst);

/* 初始化配置
 * pIniPath                 ini 文件路径
 * pParam                   存放配置的结构体指针
 */
void ReadHexViewIniFile(HANPCSTR pIniPath, void* pParam);

/* 保存配置
 * pIniPath                 ini 文件路径
 * hHexView                 窗口句柄
 */
void WriteHexViewIniFile(HANPCSTR pIniPath, HWND hHexView);

void BinDataToHexFile(HANDLE hFile, uint32_t cStartAddr, uint8_t* pBinData, uint32_t nLen);

void LinkHexFile(HANPCSTR pFileDest, HANPSTR pFileSrc, uint32_t nLen);

HANERROR HANOpenHexFile(PHEXFILE pHexFile, HANPCSTR pFileName, HANDLE hHeap);
HANERROR HexViewHexToBin(PHEXFILE pHexFile, PADDRMAP pAddrMap, HANDLE hHeap);

#ifdef __cplusplus
}
#endif

#endif
