#ifndef HAN_TOOL_BOX_TYPEDEF_H
#define HAN_TOOL_BOX_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "..\HAN_Lib\HAN_windows.h"

#define INI_MAIN_VALUE_STR_SIZE         260

#define TOOL_BOX_TEXT_INPUT_EDIT_H      26
#define TOOL_BOX_PATH_INPUT_EDIT_W      400

#define LINK_FILE_CNT_MAX               64

#define ADDR_STR_SIZE                   32
#define ADDR_PRINT_FORMAT_SINGLE        HANSIZE_PRINT_HEX_FORMAT
#define ADDR_PRINT_FORMAT_FULL          HANSIZE_PRINT_HEX8_FORMAT

#define SYS_TEXT_W                      87
#define SYS_TEXT_H                      21

typedef struct tagADDRMAP {
    uint32_t            cStartAddr;
    uint32_t            cEndAddr;
} ADDRMAP, * PADDRMAP;

typedef struct tagHANDATAINI {
    HANCHAR             pKey[INI_MAIN_VALUE_STR_SIZE];
    HANCHAR             pDefValue[INI_MAIN_VALUE_STR_SIZE];
    HANCHAR             pValue[INI_MAIN_VALUE_STR_SIZE];
} HANDATAINI, * PHANDATAINI;

typedef struct tagTOOLBOXINFO {
    HANCHAR             pTitle[256];
    HANCHAR             pClass[CLASS_STR_SIZE];
    HWND                hItem;
    HMENU               nWinId;
    void                (*RegisterHANTabWindow)(HINSTANCE hInst);
    void                (*ReadIniFile)(HANPCSTR pIniName, void* pIni);
    void                (*WriteIniFile)(HANPCSTR pIniName, HWND hTool);
    void*               pIni;
    HACCEL              (*TranslateAcceleratorCallback)(HWND hWnd);
} TOOLBOXINFO;

#ifdef __cplusplus
}
#endif

#endif
