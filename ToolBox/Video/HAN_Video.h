#ifndef HAN_VIDEO_H
#define HAN_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\ToolBoxTypedef.h"

#define HAN_VIDEO_CLASS                 TEXT("HAN_Video")

void RegisterHANVideo(HINSTANCE hInst);

/* 初始化配置
 * pIniPath                 ini 文件路径
 * pParam                   存放配置的结构体指针
 */
void ReadVideoIniFile(HANPCSTR pIniPath, void* pParam);

/* 保存配置
 * pIniPath                 ini 文件路径
 * hPicture                 窗口句柄
 */
void WriteVideoIniFile(HANPCSTR pIniPath, HWND hPicture);

#ifdef __cplusplus
}
#endif

#endif
