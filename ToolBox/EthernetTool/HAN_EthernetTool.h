#ifndef HAN_ETHERNET_TOOL_H
#define HAN_ETHERNET_TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#include "HAN_EthernetToolDef.h"

#define HAN_ETHERNET_TOOL_CLASS         TEXT("EthernetTool")

void RegisterEthernetTool(HINSTANCE hInst);

/* 初始化配置
 * pIniPath                 ini 文件路径
 * pParam                   存放配置的结构体指针
 */
void ReadEthernetToolIniFile(HANPCSTR pIniPath, void* pParam);

/* 保存配置
 * pIniPath                 ini 文件路径
 * hEthernetTool            窗口句柄
 */
void WriteEthernetToolIniFile(HANPCSTR pIniPath, HWND hEthernetTool);

#ifdef __cplusplus
}
#endif

#endif
