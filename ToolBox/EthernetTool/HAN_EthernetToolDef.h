#ifndef HAN_ETHERNET_TOOL_DEF_H
#define HAN_ETHERNET_TOOL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\GlobalVariables.h"

#define INI_ETHERNET_TOOL_VALUE_SIZE        INI_MAIN_VALUE_STR_SIZE
#define INI_ETHERNET_TOOL_APP_NAME          TEXT("EtConfig")

#define ETHERNETTOOL_BUF_SIZE               65535

#define ETHERNETTOOL_TEXT_WINDOW_LIMIT      80000
#define ETHERNETTOOL_TEXT_WINDOW_MIN_W      800
#define ETHERNETTOOL_TEXT_WINDOW_MIN_H      400

#define ETHERNETTOOL_URL_BUF_SIZE           100

#define ETHERNETTOOL_CONSOLE_WINDOW_W       750
#define ETHERNETTOOL_CONSOLE_WINDOW_H       190

#define ETHERNETTOOL_WINDOW_DX              10
#define ETHERNETTOOL_WINDOW_DY              10

#define ETHERNETTOOL_WINDOW_TEXT_HEIGHT     24

#define ETHERNETTOOL_LR_TITLE_X             ETHERNETTOOL_WINDOW_DX
#define ETHERNETTOOL_LR_TITLE_W             40

#define ETHERNETTOOL_IPV4_X                 (ETHERNETTOOL_LR_TITLE_X + ETHERNETTOOL_LR_TITLE_W)
#define ETHERNETTOOL_IPV4_W                 150

#define ETHERNETTOOL_PORT_X                 (ETHERNETTOOL_IPV4_X + ETHERNETTOOL_IPV4_W + 5)
#define ETHERNETTOOL_PORT_W                 70

#define ETHERNETTOOL_SYS_IP_X               (ETHERNETTOOL_PORT_X + ETHERNETTOOL_PORT_W + ETHERNETTOOL_WINDOW_DX)
#define ETHERNETTOOL_SYS_IP_W               78

#define ETHERNETTOOL_PROTOCOL_W             (ETHERNETTOOL_SYS_IP_X + ETHERNETTOOL_SYS_IP_W - ETHERNETTOOL_LR_TITLE_X)

#define ETHERNETTOOL_COMBOBOX_STYLE         (WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST)

/* HTTP */
#define ETHERNETTOOL_HTTP_BUTTON_W          150
#define ETHERNETTOOL_HTTP_BUTTON_H          40

#define ETHERNETTOOL_HTTP_URL_TITLE_X       ETHERNETTOOL_WINDOW_DX
#define ETHERNETTOOL_HTTP_URL_TITLE_W       40

#define ETHERNETTOOL_HTTP_URL_INPUT_X       (ETHERNETTOOL_HTTP_URL_TITLE_X + ETHERNETTOOL_HTTP_URL_TITLE_W + 3)
#define ETHERNETTOOL_HTTP_URL_INPUT_W       200

#define ETHERNETTOOL_HTTP_TEXT_X            460
#define ETHERNETTOOL_HTTP_TEXT_Y            ETHERNETTOOL_WINDOW_DY
#define ETHERNETTOOL_HTTP_TEXT_MIN_W        400
#define ETHERNETTOOL_HTTP_TEXT_MIN_H        400

/* SOME/IP */
#define ETHERNETTOOL_SOMEIP_LM_TITLE_W      90
#define ETHERNETTOOL_SOMEIP_SD_CFG_W        (ETHERNETTOOL_SOMEIP_LM_TITLE_W + ETHERNETTOOL_IPV4_W + ETHERNETTOOL_PORT_W)

#define ETHERNETTOOL_SOMEIP_BUTTON_W        150
#define ETHERNETTOOL_SOMEIP_BUTTON_H        40

typedef uint16_t                            ETHERNETPORT;
typedef const SOCKADDR_IN*                  PCSOCKADDR_IN;

typedef enum {
    WID_ETHERNET_TOOL_BASE = HAN_WID_BASE,
    WID_ETHERNET_TOOL_TEXT,
    /* 控制台 */
    WID_ETHERNET_TOOL_CONSOLE,
    WID_ETHERNET_TOOL_LOCAL_TITLE,
    WID_ETHERNET_TOOL_LOCAL_IPV4,
    WID_ETHERNET_TOOL_LOCAL_PORT,
    WID_ETHERNET_TOOL_SYS_IP,
    WID_ETHERNET_TOOL_REMOTE_TITLE,
    WID_ETHERNET_TOOL_REMOTE_IPV4,
    WID_ETHERNET_TOOL_REMOTE_PORT,
    WID_ETHERNET_TOOL_PROTOCOL,
    WID_ETHERNET_TOOL_OPEN_PORT,
    WID_ETHERNET_TOOL_CLEAR,
    WID_ETHERNET_TOOL_SEND_DATA_EDIT,
    WID_ETHERNET_TOOL_SEND,
    WID_ETHERNET_TOOL_HEX,
    /* HTTP */
    WID_ETHERNET_TOOL_HTTP,
    WID_ETHERNET_TOOL_HTTP_URL_TITLE,
    WID_ETHERNET_TOOL_HTTP_URL_INPUT,
    WID_ETHERNET_TOOL_HTTP_URL_TO_IP_BUTTON,
    WID_ETHERNET_TOOL_HTTP_IP,
    WID_ETHERNET_TOOL_HTTP_GET,
    WID_ETHERNET_TOOL_HTTP_TEXT,
    /* SOME/IP */
    WID_ETHERNET_TOOL_SOME_IP,
    WID_ETHERNET_TOOL_SOME_IP_TAB,
    /* SOME/IP Client */
    WID_ETHERNET_TOOL_SOME_IP_CLIENT,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_LOCAL_TITLE,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_LOCAL_IPV4,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_LOCAL_PORT,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_MULTI_TITLE,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_MULTI_IPV4,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_MULTI_PORT,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_FIND_SERVICE,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_SERVICE_LIST_TITLE,
    WID_ETHERNET_TOOL_SOME_IP_CLIENT_SERVICE_LIST,
    /* SOME/IP Server */
    WID_ETHERNET_TOOL_SOME_IP_SERVER,
    WID_ETHERNET_TOOL_SOME_IP_SERVER_START,
} WIDETHERNETTOOL;

typedef enum {
    INI_ETHERNET_TOOL_LOCAL_IPV4,
    INI_ETHERNET_TOOL_LOCAL_PORT,
    INI_ETHERNET_TOOL_REMOTE_IPV4,
    INI_ETHERNET_TOOL_REMOTE_PORT,
    INI_ETHERNET_TOOL_PROTOCOL,
    INI_ETHERNET_TOOL_HEX,
    INI_ETHERNET_TOOL_CFG_CNT,
} INIETHERNETTOOLCFGID;

typedef enum {
    ETHERNETTOOL_PROTOCOL_TCP_SERVER,
    ETHERNETTOOL_PROTOCOL_TCP_CLIENT,
    ETHERNETTOOL_PROTOCOL_UDP,
    ETHERNETTOOL_PROTOCOL_CNT,
} ETHERNETTOOLPROTOCOLID;

typedef struct tagETHERNETTOOLCFG {
    HANDATAINI          pSysConfig[INI_ETHERNET_TOOL_CFG_CNT];
} ETHERNETTOOLCFG, * PETHERNETTOOLCFG;

typedef struct tagETHERNETIPV4 {
    HWND                hTitle;
    HWND                hIPv4;
    HWND                hPort;
    uint8_t             pIPv4[4];
    ETHERNETPORT        nPort;
    SOCKADDR_IN         sockAddr;
    SOCKET              sct;
} ETHERNETIPV4, * PETHERNETIPV4;
typedef struct tagETHERNETEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    HWND                    hConsole;
    HWND                    hOpenPort;
    HWND                    hHttp;
    HWND                    hSomeIP;
    HANLIST                 listChildModel;
    struct {
        ETHERNETIPV4        local;
        ETHERNETIPV4        remote;
        HWND                hSysIP;
        struct {
            HWND            hWnd;
            int             eId;
        } protocol;
        int                 wsaError;
        HANDLE              hSocketThread;
    } cfgEthernet;
    struct {
        HWND                hHex;
        BOOL                bHex;
    } printHex;             /* HEX 打印串口数据 */
    struct {
        HANSIZE             nTextWindowLen;
        struct {
            HWND            hText;
            HWND            hClear;
        } printData;
    } printData;
    struct {
        HWND                hEdit;
        HWND                hSend;
        void*               pBuf;
        HANSIZE             nBufLen;
        HANPSTR             pText;
    } sendData;
    struct {
        CHAR                pBuf[ETHERNETTOOL_BUF_SIZE];
        HANCHAR             pText[(ETHERNETTOOL_BUF_SIZE * 3) + 1];
    } revData;
    struct {
        HFONT               hHex;
        HFONT               hSys;
    } hFont; 
    ETHERNETTOOLCFG         etConfig;
} ETHERNETTOOLEXTRA, * PETHERNETTOOLEXTRA;
typedef const ETHERNETTOOLEXTRA* PCETHERNETTOOLEXTRA;

typedef struct tagETHERNETTOOLREADWRITECFG {
    HANPCSTR                pKey;
    HANPCSTR                pDefValue;
    void                    (*CfgWindowToText)(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);
    void                    (*CfgTextToWindow)(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);
} ETHERNETTOOLREADWRITECFG;

typedef struct tagETHERNETTOOLPROTOCOL {
    HANPCSTR                pName;
    BOOL                    bLocalIPOptional;
    BOOL                    bLocalPortOptional;
    BOOL                    bRemoteIPOptional;
    BOOL                    bRemotePortOptional;
    BOOL                    (*CreateSocketCallback)(PETHERNETTOOLEXTRA etInfo);
} ETHERNETTOOLPROTOCOL;

#ifdef __cplusplus
}
#endif

#endif
