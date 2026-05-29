/* IFD 解码工具
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TIFF：
 * 一般来讲，JPEG 的 Exif 数据都是以 TIFF 中嵌套 IFD 的格式存储
 * TIFF 固定 8 个字节，格式如下：
 *          [2Byte 字节序][2Byte 固定内容][4Byte 首个IFD数据块的位置]
 * 字节序：
 *      字节序的两个字节只有如下两种情况：
 *      "II"（0x49,0x49）：小端序，即 LSB，低字节在前
 *      "MM"（0x4D,0x4D）：大端序，即 MSB，高字节在前
 *      后续所有内容都按这里识别到的字节序读，包括 TIFF 剩下的 6 个字节和 IFD 的数据
 * 固定内容：
 *      根据前面的字节序读取接下来的两个字节，应当读到固定值 0x002A
 *      如果字节序是小端序，那么这里读到的两个字节应该是 0x2A, 0x00 的顺序
 *      如果字节序是大端序，那么这里讲到的两个字节应该是 0x00, 0x2A 的顺序
 * 首个 IFD 位置：
 *      首个 IFD 数据块相对于 TIFF 首地址的偏移量（即字节序的第一个 Byte），后续包括 IFD 数据里提到的位置都是相对于此的偏移量
 *      理论上这里可以是大于 8 的任何位置，但因为 TIFF 头的长度固定只有 8 个字节，IFD 一般都会紧随其后，所以这里一般固定为 8
 *      如果字节序是小端序，那么这里读到的两个字节应该是 0x08, 0x00, 0x00, 0x00 的顺序
 *      如果字节序是大端序，那么这里讲到的两个字节应该是 0x00, 0x00, 0x00, 0x08 的顺序
 * 关于字节序，下文不再复述，IFD 的所有数据读取都遵循该规则，例外会特别说明
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * IFD 数据块：
 * 一个 IFD 数据块包含以下内容
 *          [2Byte 元数据个数n][元数据1][元数据2]...[元数据n][4Byte 下一个IFD数据块位置][额外数据]
 * 元数据个数n：
 *      字面意思，后面跟着 n 个 IFD 元数据
 * 元数据n：
 *      参考下文 IFD 元数据格式
 * 下一个 IFD 数据块位置：
 *      允许存在多个连续的 IFD 数据块，它们以类似链表的形式，通过数据块最后 4 个比特的 “下一个IFD数据块位置” 来链接
 *      如果位置大于 0，表示还有下一个 IFD 数据块
 *      如果位置等于 0，表示这段连续的 IFD 数据块到些结束
 * 额外数据：
 *      用来存放元数据本身不够存储的数据，大小不固定
 *      一般来讲这些数据会被 “下一个IFD数据块位置” 直接跳过，无需关心这段数据具体有多少个字节，理论上它们甚至可以分散分布在文件的各个地方
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * IFD 元数据：
 * IFD 数据块的最小单元，元数据本身大小固定，格式如下：
 *          [2Byte 标签][2Byte 数据类型][4Byte 数据个数][4Byte 数据/位置]
 * 标签：
 *      类似于 JPEG 文件的 FFD8、FFE1 之类用来表示数据功能的标签
 *      标签的具体名称和功能可查阅网址 https://exiftool.org/TagNames/EXIF.html
 * 数据类型：
 *      声明数据的类型
 * 数据个数：
 *      以数据类型为单位的数据个数，如果数据类型是 uint16_t，数据个数为 3，那么数据的总字节数就等于 6
 * 数据/位置：
 *      如果数据的总字节数 ≤ 4，那么这里存放的就是具体的数据
 *      如果数据的总字节数 > 4，那么这里存放的就是数据的位置，也就是相对于 TIFF 头的偏移量，具体的数据会放在 “IFD 数据块” 的 “额外数据” 中
 * 在读取数据时，以类型为单位，单个类型是按照 TIFF 头读到的字节序来读的，但在连续读取 n 个单位的数据时，还是按照正常顺序来读
 * 例： 如果字节序是小端序，数据类型是 uint16_t，数据个数是 3，在文件中存放的 6 个字节的数据为 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
 *      那么用 uint16_t[3] 型变量接收到的结果应为：[0] = 0x0201, [1] = 0x0403, [2] = 0x0605
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 整个 TIFF 文件形式会表现为如下：
 *      [II/MM][002A][Pos0 = 8][从 Pos0 开始的 IFD0 元数据个数 n][...n个元数据...][Pos1][IFD0 的额外数据][从 Pos1 开始的 IFD1]...
 *      ^ 所有位置都是相对于这里的偏移量
 */
#ifndef HAN_PICTURE_JPEG_IFD_H
#define HAN_PICTURE_JPEG_IFD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_PictureJpegDef.h"

#define PICTURE_JPEG_APP1_EXIF_IFD_TAG_LEN          16

typedef union {
    uint32_t    rational64u[2];
    int32_t     rational64s[2];
} PICTUREJPEGIFDTYPERATIONAL;

typedef struct tagPICTUREJPEGREADBYTES {
    uint16_t                        (*Read2Bytes)(const uint8_t pData[2]);
    uint32_t                        (*Read4Bytes)(const uint8_t pData[4]);
    uint64_t                        (*Read8Bytes)(const uint8_t pData[8]);
} PICTUREJPEGREADBYTES, * PPICTUREJPEGREADBYTES;
typedef const PICTUREJPEGREADBYTES* PCPICTUREJPEGREADBYTES;

typedef struct tagPICTUREJPEGIFDMETA {
    uint16_t                        cTag;
    uint16_t                        cDataType;
    uint32_t                        nDataCnt;
    union {
        uint8_t                     pDataU8[4];
        uint16_t                    pDataU16[2];
        uint32_t                    pDataU32[1];
        int32_t                     pDataS32[1];
        uint32_t                    nOffset;
    } cValue;
} PICTUREJPEGIFDMETA, * PPICTUREJPEGIFDMETA;

typedef struct tagPICTUREJPEGSEGMENTIFDDATA {
    PCPICTUREJPEGREADBYTES          ReadBytes;
    HANSIZE                         nMetaId;        // 对于每个不同的 IFD，都会有一个独立的元数据计数器用以计录所属 IFD 已打印了多少行数据。累加操作由用户的打印回调函数自行管理
    PICTUREJPEGIFDMETA              ifdMeta;
    const uint8_t*                  pTIFFData;      // 指向整个TIFF文件首地址
    HANSIZE                         nFileLen;
    const uint8_t*                  pIFDData;       // 如果是通过递归解析新的IFD数据，指向新的IFD头。如果通过回调函数解析IFD的数据，指向数据首地址
    HANSIZE                         nIFDLen;
    uint16_t                        cIFDID;
    HANSIZE                         nIFDBlockCnt;
    HANCHAR                         pName[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANCHAR                         pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANCHAR                         pIFDTitle[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    struct {
        const void*                 pCameraModel;
        HANSIZE                     nCmaeraModelId;
    } exInfo;
} PICTUREJPEGSEGMENTIFDDATA, * PPICTUREJPEGSEGMENTIFDDATA;

typedef struct tagPICTUREJPEGSEGMENTPRINTIFD {
    void                            (*PrintIFDTitleCallback)(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);
    void                            (*PrintIFDDataCallback)(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);
    void*                           pPrintParam;
} PICTUREJPEGSEGMENTPRINTIFD, * PPICTUREJPEGSEGMENTPRINTIFD;

typedef struct tagPICTUREJPEGSEGMENTREADEXIF {
    PICTUREJPEGSEGMENTIFDDATA       ifdData;
    PICTUREJPEGSEGMENTPRINTIFD      printIFD;
    BOOL                            (*DecodeIFDMetaCallback)(struct tagPICTUREJPEGSEGMENTREADEXIF* pReadExif);
} PICTUREJPEGSEGMENTREADEXIF, * PPICTUREJPEGSEGMENTREADEXIF;
typedef BOOL (*DECODEIFDMETACALLBACK)(PPICTUREJPEGSEGMENTREADEXIF pReadExif);

void UpdateSegmentInfoWindow_IFD_ExifReadTIFF(
    const uint8_t* pData,
    HANSIZE nLen,
    PPICTUREJPEGSEGMENTPRINTIFD pPrintIFD,
    DECODEIFDMETACALLBACK DecodeIFDMeataCallback
);

void UpdateSegmentInfoWindow_IFD_ExifReadIFDMain(
    const uint8_t* pData,
    HANSIZE nLen,
    PCPICTUREJPEGREADBYTES ReadBytesCallback,
    PPICTUREJPEGSEGMENTPRINTIFD printIFD,
    DECODEIFDMETACALLBACK DecodeIFDMeataCallback
);
void PictureJpegReadIFD(PPICTUREJPEGSEGMENTREADEXIF pReadExif);

void UpdateSegmentInfoWindow_IFD_PrintIFDRootTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);
void UpdateSegmentInfoWindow_IFD_PrintIFDStdData(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);

static inline void UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    pReadExif->printIFD.PrintIFDDataCallback(&(pReadExif->ifdData), pReadExif->printIFD.pPrintParam);
}

#ifdef __cplusplus
}
#endif

#endif
