#ifndef HAN_COM_TOOL_GRAPHIC_MAIN_H
#define HAN_COM_TOOL_GRAPHIC_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_ComToolDef.h"
#include "HAN_ComToolGraphicTypedef.h"

typedef enum {
    MSGID_COM_TOOL_GRAPHIC_NOT_USED = WM_USER,
    MSGID_COM_TOOL_GRAPHIC_UPDATE_MSG_INFO,
    MSGID_COM_TOOL_GRAPHIC_GET_DATA_OPERATION,
} COMTOOLGRAPHICMSGID;

#define HAN_COM_TOOL_GRAPHIC_MAIN_CLASS     TEXT("HAN_ComToolGraphicMain")

void RegisterHANComToolGraphicMain(HINSTANCE hInst);

void HANComToolGraphicUpdateMsg(HWND hComToolGraphicMain, const void* pData, HANSIZE nLen);

#ifdef __cplusplus
}
#endif

#endif
