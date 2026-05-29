#ifndef HAN_COM_TOOL_GRAPHIC_TYPEDEF_H
#define HAN_COM_TOOL_GRAPHIC_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

// #define _UNICODE
// #define UNICODE

#include <tchar.h>
#include <stdint.h>
#include <Windows.h>

#define Ctg_snprintf            _sntprintf
#define Ctg_strlen              _tcslen

typedef TCHAR                   CTGCHAR;
typedef TCHAR*                  PCTGSTR;
typedef const TCHAR*            PCCTGSTR;
typedef TCHAR**                 PCTGMSGSTR;

typedef struct tagCOMTOOLGRAPHICVAR {
    PCTGSTR                     pName;
    PCTGSTR                     pValue;
} COMTOOLGRAPHICVAR, * PCOMTOOLGRAPHICVAR;

typedef struct tagCOMTOOLGRAPHIC {
    PCCTGSTR                    pMsgName;
    PCOMTOOLGRAPHICVAR          pVar;
    size_t                      nVarCnt;
    size_t                      (*ReadMessage)(const uint8_t* pMsg, size_t nMsgLen, PCOMTOOLGRAPHICVAR pVar);
} COMTOOLGRAPHICMSG, * PCOMTOOLGRAPHICMSG;

typedef struct tagCOMTOOLGRAPHICINFO {
    PCOMTOOLGRAPHICMSG          pMsg;
    size_t                      nMsgCnt;
} COMTOOLGRAPHICMSGINFO, * PCOMTOOLGRAPHICMSGINFO;

#ifdef __cplusplus
}
#endif

#endif
