#ifndef HAN_ETHERNET_TOOL_CONSOLE_H
#define HAN_ETHERNET_TOOL_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

#include "..\..\HAN_EthernetToolDef.h"

void RegisterHANEthernetToolConsole(HINSTANCE hInst);

HWND CreateEthernetToolConsoleWindow(HANINT x, HANINT y, HWND hEthernetTool, HMENU winId, HINSTANCE hInst, PETHERNETTOOLEXTRA etInfo);

void ReadEthernetToolConsoleIniFile(HANPCSTR pIniPath, void* pParam);

void WriteEthernetToolConsoleIniFile(HANPCSTR pIniPath, HWND hEthernetTool);

void LoadEthernetToolCfg(PETHERNETTOOLEXTRA etInfo, PETHERNETTOOLCFG pEtConfig);

#ifdef __cplusplus
}
#endif

#endif
