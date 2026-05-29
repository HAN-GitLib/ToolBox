#ifndef HAN_TOOL_BOX_MAIN_H
#define HAN_TOOL_BOX_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\HAN_Lib\HAN_windows.h"

LRESULT CALLBACK ToolBoxMainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL ToolBoxTranslateAcceleratorCallback(PMSG pMsg);

#ifdef __cplusplus
}
#endif

#endif
