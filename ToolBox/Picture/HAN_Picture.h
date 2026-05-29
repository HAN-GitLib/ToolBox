#ifndef HAN_PICTURE_H
#define HAN_PICTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\ToolBoxTypedef.h"

#define HAN_PICTURE_CLASS               TEXT("HAN_Picture")

void RegisterHANPicture(HINSTANCE hInst);

/* 初始化配置
 * pIniPath                 ini 文件路径
 * pParam                   存放配置的结构体指针
 */
void ReadPictureIniFile(HANPCSTR pIniPath, void* pParam);

/* 保存配置
 * pIniPath                 ini 文件路径
 * hPicture                 窗口句柄
 */
void WritePictureIniFile(HANPCSTR pIniPath, HWND hPicture);

/* 获取快捷键表 */
HACCEL TranslatePictureAccelerator(HWND hPicture);

#ifdef __cplusplus
}
#endif

#endif
