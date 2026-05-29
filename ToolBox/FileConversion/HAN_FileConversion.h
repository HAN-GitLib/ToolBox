#ifndef HAN_FILE_CONVERSION_H
#define HAN_FILE_CONVERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAN_FileConversionTypedef.h"

#define FILE_CONVERSION_PRINT_SIZE_FORMAT   "%I64u"
#define FILE_CONVERSION_PRINT_OK_TEXT       "解析完成，文件大小 " FILE_CONVERSION_PRINT_SIZE_FORMAT " Bytes\r\n"\
                                            "有效字节数 " FILE_CONVERSION_PRINT_SIZE_FORMAT " Bytes，报文数 " FILE_CONVERSION_PRINT_SIZE_FORMAT "\r\n"
#define FILE_CONVERSION_PRINT_NO_DATA       "无有效数据\r\n"

typedef enum {
    FILECONVERSION_ERRNO_OK,
    FILECONVERSION_ERRNO_OPEN_DLL_FAIL,
    FILECONVERSION_ERRNO_MSG_CNT_ERR,
    FILECONVERSION_ERRNO_NOT_COMPATIBLE,
} FILECONVERSIONERRNO;

typedef struct tagFILECONVERSIONPARAM {
    void*               pSrcData;                                                           // 源数据
    size_t              nSrcBufSize;                                                        // 源数据缓存大小
    size_t              nSrcDataSize;                                                       // 源数据实际大小
    void*               pDestFile;                                                          // 存放解析后的内容的文件指针

    PFILECONVERSIONINFO pConvertMsgInfo;                                                    // 协议信息
    int*                pFilter;                                                            // 协议过滤
    PUSERSETTING        pUserSetting;                                                       // 用户自定义设置
    
    void                (*WriteFile)(void* pData, HANSIZE nLen, void* pFile);               // 写文件的接口

    void                (*PrintReport)(PCFCSTR pReport, void* pUserParam);                  // 打印 LOG 的接口
    void                (*UpdateProgress)(uint32_t i, uint32_t iMax, void* pUserParam);     // 更新进度的接口
    void*               pUserParam;                                                         // 用户自定义参数
} FILECONVERSIONPARAM, * PFILECONVERSIONPARAM;

void ConvertFile(PFILECONVERSIONPARAM pParam);

#ifdef __cplusplus
}
#endif

#endif
