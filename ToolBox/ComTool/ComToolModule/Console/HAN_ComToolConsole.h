#ifndef HAN_COM_TOOL_CONSOLE_H
#define HAN_COM_TOOL_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#include "..\..\HAN_ComToolDef.h"

void RegisterHANComToolConsole(HINSTANCE hInst);

HWND CreateComToolConsoleWindow(HANINT x, HANINT y, HWND hComTool, HMENU winId, HINSTANCE hInst, PCOMTOOLEXTRA ctInfo);

void ReadComToolConsoleIniFile(HANPCSTR pIniPath, void* pParam);

void WriteComToolConsoleIniFile(HANPCSTR pIniPath, HWND hComTool);

void LoadComToolCfg(PCOMTOOLEXTRA ctInfo, PCOMTOOLCFG pCtConfig);

#ifdef __cplusplus
}
#endif

#endif
