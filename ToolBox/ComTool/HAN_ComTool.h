#ifndef HAN_COM_TOOL_H
#define HAN_COM_TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#include "HAN_ComToolDef.h"

#define HAN_COM_TOOL_CLASS      TEXT("ComTool")

void RegisterHANComTool(HINSTANCE hInst);

/* 初始化配置
 * pIniPath                 ini 文件路径
 * pParam                   存放配置的结构体指针
 */
void ReadComToolIniFile(HANPCSTR pIniPath, void* pParam);

/* 保存配置
 * pIniPath                 ini 文件路径
 * hComTool                 窗口句柄
 */
void WriteComToolIniFile(HANPCSTR pIniPath, HWND hComTool);

#ifdef __cplusplus
}
#endif

#endif
