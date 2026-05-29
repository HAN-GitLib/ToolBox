#ifndef HAN_COM_TOOL_GRAPHIC_H
#define HAN_COM_TOOL_GRAPHIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_ComToolDef.h"
#include "HAN_ComToolGraphicTypedef.h"

typedef enum {
    COMTOOLGRAPHIC_ERRNO_OK,
    COMTOOLGRAPHIC_ERRNO_OPEN_DLL_FAIL,
    COMTOOLGRAPHIC_ERRNO_NOT_COMPATIBLE,
} COMTOOLGRAPHICERRNO;

void RegisterHANComToolGraphic(HINSTANCE hInst);

HWND CreateComToolGraphicWindow(HANINT x, HANINT y, HWND hComTool, HMENU winId, HINSTANCE hInst, PCOMTOOLEXTRA ctInfo);

void HANComToolGraphicGetDataOperation(PCOMTOOLEXTRA ctInfo);

#ifdef __cplusplus
}
#endif

#endif
