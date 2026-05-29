#ifndef HAN_FILE_CONVERSION_WINDOW_H
#define HAN_FILE_CONVERSION_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAN_FileConversionTypedef.h"
#include "HAN_FileConversion.h"

#define HAN_FILE_CONVERSION_CLASS       TEXT("HAN_FileConversion")

#define HAN_FILE_CONVERSION_PATH_MAX    260

#define USER_SETTING_KEY_NAME           TEXT("UserConfig")

typedef enum {
    INI_FILE_CONVERSION_DLL_PATH,
    INI_FILE_CONVERSION_CONVERT_PATH,
    INI_FILE_CONVERSION_CONVERT_FOLDER,
    INI_FILE_CONVERSION_FILTER,
    INI_FILE_CONVERSION_CFG_CNT,
} INIFILECONVERSIONCFGID;

typedef struct tagFILECONVERSIONCFG {
    HANDATAINI          pSysConfig[INI_FILE_CONVERSION_CFG_CNT];
    USERSETTING         usUserConfig;
} FILECONVERSIONCFG, * PFILECONVERSIONCFG;
typedef const FILECONVERSIONCFG* PCFILECONVERSIONCFG;

typedef struct tagFILECONVERSIONWINPARAM {
    FILECONVERSIONPARAM     fcParam;
    HINSTANCE               hDll;
    HANCHAR                 pDllPath[PATH_STR_SIZE];
} FILECONVERSIONWINPARAM, * PFILECONVERSIONWINPARAM;
typedef const FILECONVERSIONWINPARAM* PCFILECONVERSIONWINPARAM;

/* 注册文件转换窗口
 */
void RegisterHANFileConversion(HINSTANCE hInst);

/* 初始化配置
 * pIniPath                 ini 文件路径
 * pParam                   存放配置的结构体指针
 */
void ReadFileConversionIniFile(HANPCSTR pIniPath, void* pParam);

/* 保存配置
 * pIniPath                 ini 文件路径
 * hFileConversion          窗口句柄
 */
void WriteFileConversionIniFile(HANPCSTR pIniPath, HWND hFileConversion);

/* 初始化文件转换参数
 * pFcConfig                存放配置的结构体指针
 * pParam                   存放解析出来的转换参数的结构体指针
 */
void InitFileConversionParam(PCFILECONVERSIONCFG pFcConfig, PFILECONVERSIONWINPARAM pWinParam);

#ifdef __cplusplus
}
#endif

#endif
