#ifndef HAN_WIN_GDI_H
#define HAN_WIN_GDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAN_windows.h"

#if 1 /******************** 带前置文本、输入框、后置文本的输入控件 ********************/
#define HINPUT_CLASS    TEXT("HANInput")

#if 1 // 默认参数，这些参数不对外开放，仅在创建窗口时会参考
#endif

#if 1 // 窗口风格
#define HIPS_TEXT               0x00000000  // 接收任何文本，默认此风格
#define HIPS_NUMBER             0x00000001  // 只接收数字
#endif

typedef enum {
    HIPM_NOTUSED = WM_USER,
    HIPM_SETLEFTTEXTFONT,
    HIPM_SETINPUTFONT,
    HIPM_SETRIGHTTEXTFONT,
    HIPM_MOVELEFTTEXT,
    HIPM_MOVEINPUT,
    HIPM_MOVERIGHTTEXT,
    HIPM_SETLEFTTEXTTEXT,
    HIPM_SETINPUTTEXT,
    HIPM_SETRIGHTTEXTTEXT,
    HIPM_GETLEFTTEXTTEXT,
    HIPM_GETINPUTTEXT,
    HIPM_GETRIGHTTEXTTEXT,
} HANINPUTMESSAGE;

/* 注册输入控件类，应用程序在使用此控件前必须先调用该函数注册窗口类
 */
void RegisterHANInput(HINSTANCE hInstance);
/* 设置前置文本字体
 */
void HANInputSetLeftTextFont(HWND hInput, HFONT hFOnt, BOOL bRepaint);
/* 设置输入框字体
 */
void HANInputSetInputFont(HWND hInput, HFONT hFOnt, BOOL bRepaint);
/* 设置后置文本字体
 */
void HANInputSetRightTextFont(HWND hInput, HFONT hFOnt, BOOL bRepaint);
/* 移动前置文本
 */
void HANInputMoveLeftText(HWND hInput, int x, int y, int w, int h, BOOL bRepaint);
/* 移动输入框
 */
void HANInputMoveInput(HWND hInput, int x, int y, int w, int h, BOOL bRepaint);
/* 移动后置文本
 */
void HANInputMoveRightText(HWND hInput, int x, int y, int w, int h, BOOL bRepaint);
/* 设置前置文本的内容
 */
void HANInputSetLeftTextText(HWND hInput, HANPCSTR pText);
/* 设置输入框的内容
 */
void HANInputSetInputText(HWND hInput, HANPCSTR pText);
/* 设置后置文本的内容
 */
void HANInputSetRightTextText(HWND hInput, HANPCSTR pText);
/* 获取前置文本的内容
 */
void HANInputGetLeftTextText(HWND hInput, HANPSTR pText, int nMaxCnt);
/* 获取输入框的内容
 */
void HANInputGetInputText(HWND hInput, HANPSTR pText, int nMaxCnt);
/* 获取后置文本的内容
 */
void HANInputGetRightTextText(HWND hInput, HANPSTR pText, int nMaxCnt);
#endif

#if 1 /******************** 状态按钮 ********************/
#define HSTBT_CLASS     TEXT("HANStateButton")

#if 1 // 默认参数
#define HSTBT_TXET_SIZE     256 // 文本长度上限，含 '\0'
#endif

/* HSBTAction 指定了按下按钮时将执行的动作，该指针可以为 NULL
 * hStateButton 窗口句柄
 * hChildButton 子窗口句柄，即绘制出图形的按钮，控制方式同 windows.h 的默认 button 类
 * pParam       用户参数，该参数为调用 CreateWindow 的最后一个参数
 * return       将要切换的状态个数，用户可以通过返回值控制按钮的状态：进入下一个状态（返回 1）/ 保持当前状态（返回 0）/ 跳过 n 个状态（返回 n + 1）
 *              若函数指针为 NULL，则默认返回 1
 */
typedef HANSIZE (*HSTBTCALLBACK)(HWND hStateButton, HWND hChildButton, void* pParam);
typedef struct tagHSTBTSTATE {
    HANCHAR         pText[HSTBT_TXET_SIZE];
    HSTBTCALLBACK   Callback;
} HSTBTSTATE, * HPSTBTSTATE;
typedef const HSTBTSTATE* HPCSTBTSTATE;

/* 窗口消息 */
typedef enum {
    HSTBTM_NOTUSED = WM_USER,
    HSTBTM_ADDSTATE,
    HSTBTM_CLICK,
} HANSTATEBUTTONMESSAGE;

/* 注册多状态按钮类，应用程序在使用此控件前必须先调用该函数注册窗口类
 */
void RegisterHANStateButton(HINSTANCE hInstance);
/* 添加状态
 */
HANERROR HANStateButtonAddState(HWND hStateButton, HPCSTBTSTATE pState);
/* 点击
 */
void HANStateButtonClick(HWND hStateButton);
#endif

#if 1 /******************** COM ********************/
#define HCOM_CLASS      TEXT("HANCOM")

#if 1 // 默认参数，这些参数不对外开放，仅在创建窗口时会参考
#define HCOM_DEF_BUF_SIZE       10000
#define HCOM_NUM_TEXT_SIZE      100
#endif

#if 1 // 窗口风格
#define HCOMMS_MSG_BOX          0x00000001  // 启用消息框，若打开串口操作失败或设置非法参数时会打开消息提示框
#endif

#if 1 // 窗口消息
typedef enum {
    HCOMM_NOTUSED = WM_USER,    // 不使用的消息值，新消息请在下方添加
    HCOMM_SHOWOPENCOM,          // 显示打开串口按钮（WPARAM：BOOL 是否显示，LPARAM：未使用）
    HCOMM_SHOWPORTID,           // 显示端口号（WPARAM：BOOL 是否显示，LPARAM：未使用）
    HCOMM_SHOWBAUDRATE,         // 显示波特率（WPARAM：BOOL 是否显示，LPARAM：未使用）
    HCOMM_SHOWBYTESIZE,         // 显示字节大小（WPARAM：BOOL 是否显示，LPARAM：未使用）
    HCOMM_SHOWPARITY,           // 显示校验位（WPARAM：BOOL 是否显示，LPARAM：未使用）
    HCOMM_SHOWSTOPBITS,         // 显示停止位（WPARAM：BOOL 是否显示，LPARAM：未使用）
    HCOMM_MOVEOPENCOM,          // 移动打开串口按钮（WPARAM：BOOL bRepaint，LPARAM：RECT* 移动后的坐标）
    HCOMM_MOVEPORTID,           // 移动打开串口按钮（WPARAM：BOOL bRepaint，LPARAM：RECT* 移动后的坐标）
    HCOMM_MOVEBAUDRATE,         // 移动打开串口按钮（WPARAM：BOOL bRepaint，LPARAM：RECT* 移动后的坐标）
    HCOMM_MOVEBYTESIZE,         // 移动打开串口按钮（WPARAM：BOOL bRepaint，LPARAM：RECT* 移动后的坐标）
    HCOMM_MOVEPARITY,           // 移动打开串口按钮（WPARAM：BOOL bRepaint，LPARAM：RECT* 移动后的坐标）
    HCOMM_MOVESTOPBITS,         // 移动打开串口按钮（WPARAM：BOOL bRepaint，LPARAM：RECT* 移动后的坐标）
    HCOMM_OPENCOM,              // 打开串口（WPARAM：未使用，LPARAM：未使用）
    HCOMM_SETPORTID,            // 设置端口号（WPARAM：uint16_t 1 ~ 256，LPARAM：未使用）
    HCOMM_SETBAUDRATE,          // 设置波特率（WPARAM：DWORD 波特率，LPARAM：未使用）
    HCOMM_SETBYTESIZE,          // 设置字节大小（WPARAM：BYTE 字节大小，LPARAM：未使用）
    HCOMM_SETPARITY,            // 设置校验位（WPARAM：BYTE 校验位（NOPARITY、ODDPARITY、EVENPARITY、MARKPARITY、SPACEPARITY），LPARAM：未使用）
    HCOMM_SETSTOPBITS,          // 设置停止位（WPARAM：BYTE 停止位（ONESTOPBIT、ONE5STOPBITS（可能不能用）、TWOSTOPBITS），LPARAM：未使用）
    HCOMM_GETREVLEN,            // 获取接收到的长度（WPARAM：未使用，LPARAM：DWORD* 保存长度的 DWORD 型变量的地址）
    HCOMM_READDATA,             // 接收数据（WPARAM：未使用，LPARAM：指向 HANCOMREADDATA 结构体的指针）
    HCOMM_WRITEDATA,            // 发送数据（WPARAM：长度，LPARAM 准备发送的数据）
} HANCOMMESSAGE;
#endif

#if 1 // 通知码
// 打开端口后，窗口捕获到端口数据后会向父窗口发送 WM_NOTIFY 消息
// 下表是 LPNMHDR->code 可能出现的所有通知码
typedef enum {
    HCOMN_GETDATA,              // 端口收到数据，该通知码会在接收数据期间持续触发
    HCOMN_GETMSG,               // 端口收到报文，该通知码会在接收结束后再触发
    HCOMN_OVERFLOW,             // 端口接收缓存溢出，该通知码会在缓存溢出后持续触发
} HANCOMNOTIFYCODE;
#endif

typedef struct tagHCOMREADDATA {
    void*   pData;              // 接收数据的数组
    DWORD   nBufSize;           // 准备接收的长度，该值必须小于等于 pData 指向的数组大小
    DWORD*  nRevSize;           // 接收读操作实际收到的长度
} HANCOMREADDATA, * HANPCOMREADDATA;

/* 注册串口类，应用程序在使用此控件前必须先调用该函数注册窗口类
 */
void RegisterHANCOM(HINSTANCE hInstance);
/* 显示打开串口按钮
 * bShow        是否显示打开串口按钮（TRUE：显示，FALSE：隐藏）
 */
void HANCOMShowOpenCOM(HWND hCOM, BOOL bShow);
/* 显示端口号
 * bShow        是否显示端口号（TRUE：显示，FALSE：隐藏）
 */
void HANCOMShowPortId(HWND hCOM, BOOL bShow);
/* 显示波特率
 * bShow        是否显示端口号（TRUE：显示，FALSE：隐藏）
 */
void HANCOMShowBaudrate(HWND hCOM, BOOL bShow);
/* 显示字节大小
 * bShow        是否显示字节大小（TRUE：显示，FALSE：隐藏）
 */
void HANCOMShowByteSize(HWND hCOM, BOOL bShow);
/* 显示校验位
 * bShow        是否显示校验位（TRUE：显示，FALSE：隐藏）
 */
void HANCOMShowParity(HWND hCOM, BOOL bShow);
/* 显示停止位
 * bShow        是否显示停止位（TRUE：显示，FALSE：隐藏）
 */
void HANCOMShowStopBits(HWND hCOM, BOOL bShow);
void HANCOMMoveOpenCOM(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint);
void HANCOMMovePortId(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint);
void HANCOMMoveBaudrate(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint);
void HANCOMMoveByteSize(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint);
void HANCOMMoveParity(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint);
void HANCOMMoveStopBits(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint);
/* 打开串口
 */
BOOL HANCOMOpenCOM(HWND hCOM);
/* 设置端口号
 */
HANERROR HANCOMSetPortId(HWND hCOM, uint16_t nPortId);
/* 设置波特率
 */
BOOL HANCOMSetBaudrate(HWND hCOM, DWORD nBaudrate);
/* 设置字节大小
 */
BOOL HANCOMSetByteSize(HWND hCOM, BYTE nByteSize);
/* 设置校验位
 */
BOOL HANCOMSetParity(HWND hCOM, BYTE nParity);
/* 设置停止位
 */
BOOL HANCOMSetStopBits(HWND hCOM, BYTE nStopBits);
/* 获取串口缓存中收到的数据长度
 */
HANERROR HANCOMGetRevLen(HWND hCOM, DWORD* pLen);
/* 读取数据
 */
HANERROR HANCOMReadData(HWND hCOM, void* pData, DWORD nBufSize, DWORD* nRevSize);
/* 发送数据
 */
HANERROR HANCOMWriteData(HWND hCOM, const void* pData, DWORD nLen);
#endif

#if 1 /******************** 标签 ********************/
// #define HTC_CLASS      TEXT("HANTabControl")

// /* 注册多状态按钮类，应用程序在使用此控件前必须先调用该函数注册窗口类
//  */
// void RegisterHANTabControl(HINSTANCE hInstance);
#endif

#ifdef __cplusplus
}
#endif

#endif
