#ifndef HAN_FILE_CONVERSION_TYPEDEF_H
#define HAN_FILE_CONVERSION_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <tchar.h>
#include <windows.h>

#include "..\ToolBoxTypedef.h"

#define HAN_FILE_CONVERSION_INFO_MAX                32

#define FILE_CONVERSION_USER_SETTING_CFG_STR_SIZE   260
#define FILE_CONVERSION_USER_SETTING_CFG_CNT_MAX    30

#define Fc_snprintf             _sntprintf
#define Fc_strlen               _tcslen
#define Fc_strcpy               _tcscpy

typedef TCHAR                   FCCHAR;
typedef TCHAR*                  PFCSTR;
typedef const TCHAR*            PCFCSTR;

typedef enum {
    WID_FILE_CONVERSION_BASE = HAN_WID_BASE,
    WID_FILE_CONVERSION_DLL_NAME,
    WID_FILE_CONVERSION_IMPORT_DLL,
    WID_FILE_CONVERSION_EXPORT_DLL,
    WID_FILE_CONVERSION_PATH_INPUT,
    WID_FILE_CONVERSION_PATH_CHOOSE,
    WID_FILE_CONVERSION_CONVERT_BUTTON,
    WID_FILE_CONVERSION_CONVERT_DIR,
    WID_FILE_CONVERSION_FILTER,
    WID_FILE_CONVERSION_USERSETTING,
    WID_FILE_CONVERSION_PROGRESS,
    WID_FILE_CONVERSION_REPORT,
} WIDFILECONVERSION;

typedef struct tagUSERSETTING {
    FCCHAR                      pCfg[FILE_CONVERSION_USER_SETTING_CFG_CNT_MAX][FILE_CONVERSION_USER_SETTING_CFG_STR_SIZE];  // 存放配置的字符串
    uint32_t                    nCfgCnt;                                                                                    // 配置个数
    void*                       pData;                                                                                      // 配置数据，用户自由使用
} USERSETTING, * PUSERSETTING;

typedef struct tagFILECONVERSIONUSERSETTING {
    /* 初始化用户设置
     * pUserSetting             自定义设置结构体指针
     * hParentWnd               父窗口句柄，如果控制台命令行启动则为 NULL
     * hParentInst              程序实例句柄
     */
    void                        (*InitUserSetting)(PUSERSETTING pUserSetting, HWND hParentWnd, HINSTANCE hInst);
    /* 创建用户自定义设置窗口
     * pUserSetting             自定义设置结构体指针
     */
    INT_PTR                     (*UserSettingDialogProc)(HWND hUserSetting, UINT message, WPARAM wParam, LPARAM lParam);
    /* 对话框的位置 */
    RECT                        rcDialog;
    /* 保存用户自定义设置
     * pUserSetting             自定义设置结构体指针
     */
    void                        (*SaveUserSetting)(PUSERSETTING pUserSetting);
} FILECONVERSIONUSERSETTING, * PFILECONVERSIONUSERSETTING;

typedef struct tagFILECONVERSION {
    PFCSTR                      pMsgName;
    PFCSTR                      pTitle;
    uint32_t                    (*ReadMessage)(const uint8_t* pMsg, HANINT nMsgLen, PUSERSETTING pUserSetting, HANDLE hDestFile);
} FILECONVERSION, * PFILECONVERSION;

typedef struct tagFILECONVERSIONINFO {
    /* 打开文件时执行的操作
     * pSrcName                 系统打开的源文件名
     * pDestName                存放解析后的数据的文件名，由用户生成
     * return                   是否解析此文件。TRUE：解析文件，FALSE：忽略文件（解析生成新文件后可能会被系统再次打开，用户可根据文件后缀名返回 FALSE 以忽略此类文件）
     */
    bool                        (*OpenSrcFileAction)(PCFCSTR pSrcName, PFCSTR pDestName);
    /* 文件转换信息表，不必填满
     */
    FILECONVERSION              pFileConversion[HAN_FILE_CONVERSION_INFO_MAX];
    /* 文件转换信息表的实际长度
     */
    uint32_t                    nFileConversionCnt;
    /* 自定义设置信息
     */
    FILECONVERSIONUSERSETTING   fcUserSetting;
} FILECONVERSIONINFO, * PFILECONVERSIONINFO;

#ifdef __cplusplus
}
#endif

#endif
