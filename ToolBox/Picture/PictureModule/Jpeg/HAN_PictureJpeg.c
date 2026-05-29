#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <shlwapi.h>

#include "HAN_PictureJpeg.h"
#include "HAN_PictureJpegLib.h"
#include "HAN_PictureJpegMath.h"
#include "HAN_PictureJpegSegment.h"
#include "HAN_PictureJpegPaint.h"
#include "GlobalVariables\HAN_PictureJpegGlobalVariables.h"
#include "APP\IFD\HAN_PictureJpegIFD.h"
#include "MCUView\HAN_PictureJpegMCU.h"
#include "..\..\..\..\HAN_Lib\HAN_windows.h"
#include "..\..\HAN_PictureDef.h"
#include "..\..\HAN_PictureLib.h"
#include "..\EditTool\HAN_PictureEditTool.h"

#include "APP\APP0\JFIF\HAN_PictureJpegJFIF.h"
#include "APP\APP1\Exif\HAN_PictureJpegExif.h"
#include "APP\APP1\XMP\HAN_PictureJpegXMP.h"
#include "APP\APP2\MPF\HAN_PictureJpegMPF.h"
#include "APP\APP2\ICCProfile\HAN_PictureJpegICCProfile.h"
#include "APP\APP13\Photoshop\HAN_PictureJpegPhotoshop.h"
#include "APP\APP14\Adobe\HAN_PictureJpegAdobe.h"

/* YCbCr 转 RGB 的系数（浮点 * 65536） */
#define INT_1_402       91881
#define INT_0_344136    22553
#define INT_0_714136    46801
#define INT_1_772       116129

typedef enum {
    PICTURE_JPEG_SEGMENT_LIST_HEADER_MARKER,
    PICTURE_JPEG_SEGMENT_LIST_HEADER_LEN,
    PICTURE_JPEG_SEGMENT_LIST_HEADER_EXLEN,
    PICTURE_JPEG_SEGMENT_LIST_HEADER_DATA,
    PICTURE_JPEG_SEGMENT_LIST_HEADER_ADDRESS,
    PICTURE_JPEG_SEGMENT_LIST_HEADER_CNT,
} PICTUREJPEGSEGMENTLISTHEADER;

typedef enum {
    PICTURE_JPEG_SEGMENT_TYPE_SOI,
    PICTURE_JPEG_SEGMENT_TYPE_APPn,
    PICTURE_JPEG_SEGMENT_TYPE_COM,
    PICTURE_JPEG_SEGMENT_TYPE_DQT,
    PICTURE_JPEG_SEGMENT_TYPE_DHT,
    PICTURE_JPEG_SEGMENT_TYPE_SOFn,
    PICTURE_JPEG_SEGMENT_TYPE_DRI,
    PICTURE_JPEG_SEGMENT_TYPE_SOS,
    PICTURE_JPEG_SEGMENT_TYPE_RSTn,
    PICTURE_JPEG_SEGMENT_TYPE_EOI,
    PICTURE_JPEG_SEGMENT_TYPE_CNT,
} PICTUREJPEGSEGMENTTYPE;

typedef enum {
    PICTURE_JPEG_READ_BIT_RET_NROMAL,
    PICTURE_JPEG_READ_BIT_RET_RSTN,
    PICTURE_JPEG_READ_BIT_RET_END,
    PICTURE_JPEG_READ_BIT_RET_ERROR,
} PICTUREJPEGREADBITRET;

typedef enum {
    PICTURE_JPEG_DECODE_PROCEDURE_MODE_AUTO,
    PICTURE_JPEG_DECODE_PROCEDURE_MODE_SLOW,
    PICTURE_JPEG_DECODE_PROCEDURE_MODE_FASE,
    PICTURE_JPEG_DECODE_PROCEDURE_MODE_CNT,
} PICTUREJPEGDECODEPROCEDUREMODE;

typedef PICTUREJPEGNUMBER PICTUREJPEGYCBCR[3];
typedef PICTUREJPEGYCBCR* PPICTUREJPEGYCBCR;

typedef struct tagPICTUREJPEGSEGMENTHEADERINFO {
    HANPCSTR                        pName;
    HANINT                          nWidth;
} PICTUREJPEGSEGMENTHEADERINFO;

typedef struct tagPICTUREJPEGWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HWND                            hShow;
    HWND                            hEditTool;
    HBRUSH                          hBackground;
    PICTURECREATEPARAM              paramPicture;
    struct {
        HWND                        hButton;
        HWND                        hWindow;
    } mcuView;
    struct {
        HWND                        hStatic;
        HWND                        hToolTip;
    } warning;
    struct {
        HWND                        hList;
        HWND                        hInfo;
        PICTUREJPEGSEGMENTINFO      segmentInfo;    /* pSegmentList 的信息汇总到这个成员里 */
        struct {
            HANSIZE                 nCnt;           /* 文件中的 segment 总数 */
            PPICTUREJPEGSEGMENT     pSegmentList;   /* 列举了文件中每个 segment */
        } map;
    } segment;
    struct {
        PICTUREPIXELINFO            pixelInfo;
        uint8_t                     cColorType;                                     /* 颜色类型，如灰度图、真彩色图等 */
        PICTUREJPEGDCTMATRIX1D*     pDecodedData1D;                                 /* 一维数据块，ZigZag 排列过渡用 */
        PICTUREJPEGDCTMATRIX2D*     pDecodedData2D;                                 /* 二维数据块，ZigZag 排列过渡用 */
        SIZE_T                      nDecodedDataLen;                                /* 数据块的总长 */
        PICTUREJPEGDCTMATRIX2D*     pIDCTData;                                      /* 存放 IDCT 后的数据 */
        SIZE_T                      nIDCTDataLen;                                   /* IDCT 后的数据块的总长 */
        PPICTUREJPEGYCBCR           pYCbCrData;                                     /* 存放颜色数据，即反量化，IDCT，补全像素后的数据，未换算到 RGB */
        SIZE_T                      nYCbCrDataLen;                                  /* 颜色数据的长度 */
        HANPPICTUREINFO             pPictureInfo;                                   /* 指向最终的图像信息 */
        PICTUREJPEGMCUTABLE         mcuTable[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];   /* 存放各分量的 MCU 信息 */
    } pictureData;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
        HFONT                       hInfo;
    } hFont;                        /* 字体 */
    uint8_t                         pBuf[]; /* 存放 pictureData 成员所需的缓存 */
} PICTUREJPEGWNDEXTRA, * PPICTUREJPEGWNDEXTRA;

typedef struct tagPICTUREJPEGSEGMENTTYPEINFO {
    HANSIZE                         (*ReadSegment)(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);
    BOOL                            (*SetSegmentMap)(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment);
    BOOL                            (*UpdateSegmentInfoWindow)(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
} PICTUREJPEGSEGMENTTYPEINFO;

typedef struct tagPICTUREJPEGHUFFMAN {
    uint8_t                         nValue;
    uint8_t                         nLen;
    uint16_t                        cCode;
} PICTUREJPEGHUFFMAN, * PPICTUREJPEGHUFFMAN;
typedef const PICTUREJPEGHUFFMAN* PCPICTUREJPEGHUFFMAN;

typedef struct tagPICTUREJPEGREADBIT {
    const uint8_t*                      pData;
    HANSIZE                             nDataLen;
    HANCPRIVATE uint8_t                 cData;
    HANCPRIVATE HANSIZE                 iByte;
    HANCPRIVATE uint8_t                 iBit;
    HANCPUBLIC uint32_t                 cCode;
    HANCPUBLIC uint8_t                  nCodeLen;
} PICTUREJPEGREADBIT, * PPICTUREJPEGREADBIT;
typedef struct tagPICTUREJPEGWRITEBITS {
    HANDLE                              hFile;
    uint8_t*                            pData;
    HANSIZE                             nDataLen;
    HANCPRIVATE uint8_t                 cData;
    HANCPRIVATE HANSIZE                 iByte;
    HANCPRIVATE uint8_t                 iBit;
} PICTUREJPEGWRITEBITS, * PPICTUREJPEGWRITEBITS;

typedef struct tagPICTUREJPEGDECODEMACHINE {
    PICTUREJPEGREADBIT                  rbBits;
    PPICTUREJPEGSEGMENTINFO             pSegmentInfo;
    PPICTUREJPEGSEGMENTDHTINFO          pHT;
    HANSIZE                             nBlockTotal;
    HANSIZE                             nBlockId;
    HANSIZE                             nBlockSum;
    uint8_t                             nMCUComponentBlockTotal;
    uint8_t*                            pMCUComponentBlockCnt;
    uint8_t*                            pMCUComponentBlockOffset;
    uint8_t                             nCurrentComponent;
    uint8_t                             nSs;
    uint8_t                             nSe;
    uint8_t                             nAh;
    uint8_t                             nAl;
    uint8_t                             nSpectral;
    PICTUREJPEGDCTMATRIX1D*             pDecodedData1D;
    PICTUREJPEGDCTMATRIX2D*             pDecodedData2D;
    PICTUREJPEGDCTMATRIX2D*             pIDCT;
    PICTUREJPEGNUMBER                   pPredQT[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    PPICTUREJPEGMCUINFO                 pMCUInfo[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    struct {
        PICTURERESOLUTION               pPictureBlockCnt[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];   // 渐进式细化扫描每行的 Block 个数（细化扫描的 Block 按行排列，而非 MCU）
        PICTURERESOLUTION               pMCUBlockCnt[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];       // 等价于分量的采样系数
    } progressiveParam;
} PICTUREJPEGDECODEMACHINE, * PPICTUREJPEGDECODEMACHINE;

typedef struct tagPICTUREJPEGDECODEMODEINFO {
    HANPCSTR                            pText;
    void                                (*DecodeCallback)(PPICTUREJPEGWNDEXTRA jpegInfo);
} PICTUREJPEGDECODEMODEINFO;

typedef struct tagPICTUREJPEGSAVEPARAM {
    PPICTUREJPEGWNDEXTRA                jpegInfo;
    PICTUREJPEGTYPE                     jpegType;
    HANPPICTURE                         pPicture;
    uint8_t                             nComponent;
    uint8_t                             pSamplingCoe[PICTURE_JPEG_SEGMENT_COMPONENT_MAX][2];
    PICTURESAVEJPEGQUANTIZATIONTABLE    eQuantizationTable;
    HWND                                hOK;
    HWND                                hCancel;
    struct {
        HWND                            hTitle;
        HWND                            hLow;
        HWND                            hMiddle;
        HWND                            hHigh;
    } hSampleQuality;
    struct {
        HWND                            hTitle;
        HWND                            hOfficial;
        HWND                            hHigh;
    } hQuantizeQuality;
    struct {
        HWND                            hTitle;
        HWND                            hBaseline;
        HWND                            hProgressive;
    } hType;
} PICTUREJPEGSAVEPARAM, * PPICTUREJPEGSAVEPARAM;

typedef struct tagPICTUREJPEGENCODEMACHINE{
    PICTUREJPEGWRITEBITS                    wbBits;
    HANPPICTURE                             pPicture;
    PICTUREJPEGTYPE                         jpegType;
    uint8_t                                 nComponent;
    const uint8_t*                          pSamplingCoe[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    PICTURERESOLUTION                       pxMCUCnt;
    PICTURERESOLUTION                       pxMCUBlockCnt;
    PICTURERESOLUTION                       pxMCUSize;
    PICTURERESOLUTION                       pxSample;
    HANSIZE                                 nMCUTotal;
    uint8_t                                 pMCUComponentBlockCnt[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    uint8_t                                 pMCUComponentBlockOffset[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    uint8_t                                 nMCUComponentBlockTotal;
    HANSIZE                                 pComponentBlockTotal[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    HANSIZE                                 nBlockTotal;
    uint8_t                                 pBlockComponentId[PICTURE_JPEG_SEGMENT_MCU_BLOCK_MAX];
    PCPICTURESAVEJPEGQUANTIZATIONTABLEINFO  pQT;
    uint8_t                                 nQTId[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    PCPICTURESAVEJPEGHUFFMANTABLEINFO       pHT;
    uint8_t                                 nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_CNT][PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    PPICTUREJPEGYCBCR                       pYCbCr;             // 转换成 YCbCr 后的像素数据，还未采样
    PPICTUREJPEGYCBCR*                      pYCbCrMap;          // 可以通过二维数组方式访问 YCbCr 的映射表
    PICTUREJPEGDCTMATRIX2D*                 pComponentSample[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];       // 各分量按照各自采样系数采样后的数据
    PICTUREJPEGDCTMATRIX2D*                 pComponentTransform[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];    // DCT、量化、差分预测
    PICTUREJPEGDCTMATRIX1D*                 pEncodeData;        // 之字形排列并且合并了各分量后的数据，尚未编码
    uint8_t*                                pData;              // 熵编码后的数据，也就是准备写入文件的数据。相当于一个用来写入文件的缓存，无需填满，只要准备合适的大小不要频繁写文件即可
    uint8_t                                 pBuf[];
} PICTUREJPEGENCODEMACHINE, * PPICTUREJPEGENCODEMACHINE;

typedef BOOL (*DECODEPROGRESSIVEACHUFFMAN)(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode);

static LRESULT CALLBACK PictureJpegWndProc(HWND hPictureJpeg, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPictureJpeg, LPARAM lParam);
static void SizeCallback(HWND hPictureJpeg, PPICTUREJPEGWNDEXTRA jpegInfo);
static HBRUSH StaticColorCallback(PPICTUREJPEGWNDEXTRA jpegInfo, HWND hStatic);
static void CommandCallback(PPICTUREJPEGWNDEXTRA jpegInfo, WPARAM wParam);
static BOOL NotifyCallback(PPICTUREJPEGWNDEXTRA jpegInfo, NMHDR* pNotify);
static void DestroyCallback(PPICTUREJPEGWNDEXTRA jpegInfo);
static LRESULT GetSaveParamCallback(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTURESAVEPARAM pSaveParam);
static void InitJpegSegmentWindow(PPICTUREJPEGWNDEXTRA jpegInfo);
static HANINT JpegProcess(PPICTUREJPEGWNDEXTRA jpegInfo);
static PPICTUREJPEGWNDEXTRA ReallocJpegInfoMemory(PPICTUREJPEGWNDEXTRA jpegInfo);
static HANSIZE GetMCUInfoSize(PPICTUREJPEGWNDEXTRA jpegInfo);
static void InitMCUInfoMap(PPICTUREJPEGWNDEXTRA jpegInfo, uint8_t* pBuf);
static HANSIZE ReadJpegSegment(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);
static HANSIZE GetNextSegmentPos(const uint8_t* pData, HANSIZE nLen);
static void GetJpegShowSize(PPICTUREJPEGWNDEXTRA jpegInfo, HANINT* pW, HANINT* pH);
static void SetSegmentMap(const uint8_t* pData, HANINT nSegmentCnt, PPICTUREJPEGWNDEXTRA jpegInfo);
static void CreateWarningWindow(PPICTUREJPEGWNDEXTRA jpegInfo);
static void MCUViewCallback(PPICTUREJPEGWNDEXTRA jpegInfo);
static BOOL SegmentListNotifyCallback(PPICTUREJPEGWNDEXTRA jpegInfo, NMHDR* pNotify);
static BOOL UpdateSegmentInfo(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static void UpdateSegmentInfoWindow(PPICTUREJPEGWNDEXTRA jpegInfo, HANINT nId);
static void PrintHuffmanCode(PCPICTUREJPEGHUFFMAN pHuffman, HANPSTR pText, size_t nLen);

static BOOL CreateJpegSaveParamDialog(PPICTUREJPEGSAVEPARAM pParam);
static INT_PTR CALLBACK JpegSaveParamDialogProc(HWND hSaveParam, UINT message, WPARAM wParam, LPARAM lParam);
static void JpegSaveParamInitCallback(HWND hSaveParam, LPARAM lParam);
static LRESULT JpegSaveParamGroupBoxSubClassCallback(HWND hGroupBox, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR nId, DWORD_PTR pParam);
static void JpegSaveParamCreateSampleQualityWindows(PPICTUREJPEGSAVEPARAM pSaveParam);
static void JpegSaveParamCreateQuantizeQualityWindows(PPICTUREJPEGSAVEPARAM pSaveParam);
static void JpegSaveParamCreateTypeWindows(PPICTUREJPEGSAVEPARAM pSaveParam);
static void JpegSaveParamCommandCallback(HWND hSaveParam, PPICTUREJPEGSAVEPARAM pSaveParam, WPARAM wParam);
static void JpegSaveParamSetSamplingCoeLow(PPICTUREJPEGSAVEPARAM pSaveParam);
static void JpegSaveParamSetSamplingCoeMiddle(PPICTUREJPEGSAVEPARAM pSaveParam);
static void JpegSaveParamSetSamplingCoeHigh(PPICTUREJPEGSAVEPARAM pSaveParam);
static PPICTUREJPEGENCODEMACHINE AllocJpegEncodeMemory(PPICTUREJPEGSAVEPARAM pSaveParam);
static void SaveJpegWriteDQT(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void SaveJpegWriteSOFn(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void SaveJpegWriteDHT(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void SaveJpegWriteSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);

static void SaveJpegRGBToYCbCr(PPICTUREJPEGENCODEMACHINE pEncode);
static void SaveJpegSampleYCbCr(PPICTUREJPEGENCODEMACHINE pEncode);

static inline HANINT GetSegmentListWindowWidth(void);
static inline HANINT GetSegmentInfoWindowWidth(void);

static HANSIZE ReadJpegSegment_Default(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);
static HANSIZE ReadJpegSegment_SOI(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);
static HANSIZE ReadJpegSegment_SOS(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);
static HANSIZE ReadJpegSegment_RSTn(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);
static HANSIZE ReadJpegSegment_EOI(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment);

static BOOL UpdateSegmentInfo_DQT(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static BOOL UpdateSegmentInfo_DHT(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static BOOL UpdateSegmentInfo_SOFn(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static BOOL UpdateSegmentInfo_SOS(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo);

static void SetSegmentMap_Default(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment);
static BOOL SetSegmentMap_SOI_EOI(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment);
static BOOL SetSegmentMap_SOS(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment);
static BOOL SetSegmentMap_RSTn(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment);

static void UpdateSegmentInfoWindow_InsertBlankLine(HWND hListView);
static BOOL UpdateSegmentInfoWindow_Default(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_SOI(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_APPn(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_COM(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_DQT(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_DHT(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_SOFn(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_DRI(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_SOS(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_RSTn(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);
static BOOL UpdateSegmentInfoWindow_EOI(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);

static BOOL DecodeSOS(PPICTUREJPEGWNDEXTRA jpegInfo);

static BOOL DecodeBaselineSOS(PPICTUREJPEGWNDEXTRA jpegInfo);
static BOOL DecodeBaselineHuffman(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static inline BOOL DecodeBaselineDCHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode);
static inline BOOL DecodeBaselineACHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode);
static inline void DecodeRSTnCallback(PPICTUREJPEGDECODEMACHINE pDecode);
static inline void DecodeBaselineBlockOkCallback(PPICTUREJPEGDECODEMACHINE pDecode);
static inline void BaselineZigZag(PPICTUREJPEGDECODEMACHINE pDecode);
static void BaselineDequantizeBlock(PPICTUREJPEGWNDEXTRA jpegInfo);

static BOOL DecodeProgressiveSOS(PPICTUREJPEGWNDEXTRA jpegInfo);
static BOOL DecodeProgressiveDCFirstScan(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static BOOL DecodeProgressiveDCSubsequentScan(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo);
static BOOL DecodeProgressiveACScan(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo, DECODEPROGRESSIVEACHUFFMAN DecodeCallback);
static void InitProgressiveDecodeMachineParam(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGDECODEMACHINE pDecode);
static inline void DecodeProgressiveDCHuffmanValue(uint16_t nValue, uint8_t nLen, PPICTUREJPEGDECODEMACHINE pDecode);
static BOOL DecodeProgressiveACFirstScanHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode);
static BOOL DecodeProgressiveACSubsequentScanHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode);
static inline BOOL DecodeProgressiveACBlockOkCallback(PPICTUREJPEGDECODEMACHINE pDecode);
static inline void ProgressiveUpdateBlockId(PPICTUREJPEGDECODEMACHINE pDecode);
static inline void ProgressiveZigZag(PPICTUREJPEGDECODEMACHINE pDecode);
static void ProgressiveDequantizeBlock(PPICTUREJPEGWNDEXTRA jpegInfo);
static inline BOOL ProgressiveACSubsequentScanJumpRRRR(PPICTUREJPEGDECODEMACHINE pDecode, uint8_t nRRRR);
static inline BOOL ProgressiveACSubsequentScanJumpZRL(PPICTUREJPEGDECODEMACHINE pDecode);
static inline BOOL ProgressiveACSubsequentScanJumpEOBRUN(PPICTUREJPEGDECODEMACHINE pDecode, HANSIZE nEOBRUN);

static PICTUREJPEGREADBIT JpegInitReadBit(const uint8_t* pData, HANSIZE nLen);
static inline void JpegClearBit(PPICTUREJPEGREADBIT pReadBit);
static inline uint8_t JpegGetBitsLen(PPICTUREJPEGREADBIT pReadBit);
/* 读取一个比特
 * return:      PICTURE_JPEG_READ_BIT_RET_NROMAL: 非其它返回值，即正常读取一个 bit
 *              PICTURE_JPEG_READ_BIT_RET_RSTN: 如果读到了一个 byte 的最后一个 bit，并且下一个 byte 是 RSTn，返回该值，程序应在调用函数处理完 bit 之后处理 RSTn
 *              PICTURE_JPEG_READ_BIT_RET_END: 如果读到了最后一个 byte 的最后一个 bit，并且再调用一次函数，返回该值
 *              PICTURE_JPEG_READ_BIT_RET_ERROR: 如果遇到了错误的 bit 序列（如非 0xFF00，非 RSTn 的 0xFF），返回该值
 */
static inline PICTUREJPEGREADBITRET JpegReadBit(PPICTUREJPEGREADBIT pReadBit);
/* 读取多个比特
 * return:      返回读取最后一个 bit 的结果，并且前面的 bit 必须都返回 PICTURE_JPEG_READ_BIT_RET_NROMAL，否则返回 PICTURE_JPEG_READ_BIT_RET_ERROR
 */
static inline PICTUREJPEGREADBITRET JpegReadBits(PPICTUREJPEGREADBIT pReadBit, uint8_t nLen);
static inline HANSIZE GetBitCnt(HANSIZE nByteStart, uint8_t nBitStart, HANSIZE nByteEnd, uint8_t nBitEnd);
/* 查表获取 Huffman的码值
 * nCodeLen：   从 1 开始，码长为 1 就传 1，不用减 1
 */
static inline uint8_t* DecodeHuffman(uint16_t cCode, uint8_t nCodeLen, PPICTUREJPEGSEGMENTDHTINFO pHuffmanTable);
static inline int16_t DecodeVLI(uint16_t cCode, uint8_t nLen);

static void InitComponentMCUInfo(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGDECODEMACHINE pDecode, uint8_t nComponent);
static void RecordMCUInfoStart(PPICTUREJPEGDECODEMACHINE pDecode);
static void RecordMCUInfoEnd(PPICTUREJPEGDECODEMACHINE pDecode);
static inline void SwitchComponent(PPICTUREJPEGDECODEMACHINE pDecode);
static inline PPICTUREJPEGSEGMENTDHTINFO SwitchComponentAndHuffmanTable(PPICTUREJPEGDECODEMACHINE pDecode, PICTUREJPEGSEGMENTDHTTABLETYPE eDcAc);
static void MapJpegPixels(PPICTUREJPEGWNDEXTRA jpegInfo);
static void ConvertYCbCrToRGB(PPICTUREJPEGWNDEXTRA jpegInfo);
static void ConvertYCbCrToRGBFast(PPICTUREJPEGWNDEXTRA jpegInfo);
static inline uint8_t I32RGBToU8RGB(int32_t cRGB);

static void JpegDecodeModeAuto(PPICTUREJPEGWNDEXTRA jpegInfo);
static void JpegDecodeModeSlow(PPICTUREJPEGWNDEXTRA jpegInfo);
static void JpegDecodeModeFast(PPICTUREJPEGWNDEXTRA jpegInfo);

static void EncodeSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void EncodeBaselineSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void EncodeBaselineSOSHuffman(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);

static void EncodeProgressiveSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void EncodeProgressiveSOSDC(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile);
static void EncodeProgressiveSOSAC(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile, uint8_t nTargetComponent, uint8_t nSs, uint8_t nSe);
static inline void EncodeProgressiveSOWriteEOBRUN(PPICTUREJPEGENCODEMACHINE pEncode, uint16_t nEOBRUN, PCPICTUREJPEGSEGMENTDHTINFO pHTAC);

static void EncodeSOSQuantization(PICTUREJPEGDCTMATRIX2D* pBlock, const PICTUREJPEGDCTMATRIX2D* pQT, HANSIZE nLen);
static void EncodeSOSZigZag(PPICTUREJPEGENCODEMACHINE pEncode);
static inline PCPICTUREJPEGSEGMENTDHTINFO EncodeSOSGetComponentHuffmanTable(PPICTUREJPEGENCODEMACHINE pEncode, uint8_t nComponent, PICTUREJPEGSEGMENTDHTTABLETYPE eType);

static PICTUREJPEGWRITEBITS JpegInitWriteBits(uint8_t* pData, HANSIZE nLen, HANDLE hFile);
static inline void JpegWriteBits(PPICTUREJPEGWRITEBITS pWriteBits, uint16_t cData, uint8_t nLen);
static void JpegFillBits(PPICTUREJPEGWRITEBITS pWriteBits);
static inline void EncodeVLI(PICTUREJPEGNUMBER nValue, uint16_t* pCode, uint8_t* nLen);

static const PICTUREJPEGWNDEXTRA sg_jpegDefaultInfo = { 0 };
static const uint8_t sg_pJpegHeader[2] = { 0xFF, 0xD8, };
static const PICTUREJPEGSEGMENTHEADERINFO sg_pJpegSegmentListHeader[PICTURE_JPEG_SEGMENT_LIST_HEADER_CNT] = {
    [PICTURE_JPEG_SEGMENT_LIST_HEADER_MARKER] = {
        .pName = TEXT("标记"),
        .nWidth = 60,
    },
    [PICTURE_JPEG_SEGMENT_LIST_HEADER_LEN] = {
        .pName = TEXT("数据长度"),
        .nWidth = 100,
    },
    [PICTURE_JPEG_SEGMENT_LIST_HEADER_EXLEN] = {
        .pName = TEXT("附加长度"),
        .nWidth = 130,
    },
    [PICTURE_JPEG_SEGMENT_LIST_HEADER_DATA] = {
        .pName = TEXT("数据"),
        .nWidth = 350,
    },
    [PICTURE_JPEG_SEGMENT_LIST_HEADER_ADDRESS] = {
        .pName = TEXT("地址"),
        .nWidth = 100,
    },
};
static const HANINT sg_pJpegSegmentInfoHeaderWidth[PICTURE_JPEG_SEGMENT_INFO_HEADER_CNT] = {
    [PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD] = 180,
    [PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE] = 300,
};
static const PICTUREJPEGSEGMENTTYPEINFO sg_pJpegSegmentType[PICTURE_JPEG_SEGMENT_TYPE_CNT] = {
    [PICTURE_JPEG_SEGMENT_TYPE_SOI] = {
        .ReadSegment = ReadJpegSegment_SOI,
        .SetSegmentMap = SetSegmentMap_SOI_EOI,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_SOI,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_APPn] = {
        .ReadSegment = NULL,
        .SetSegmentMap = NULL,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_APPn,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_COM] = {
        .ReadSegment = NULL,
        .SetSegmentMap = NULL,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_COM,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_DQT] = {
        .ReadSegment = NULL,
        .SetSegmentMap = NULL,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_DQT,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_DHT] = {
        .ReadSegment = NULL,
        .SetSegmentMap = NULL,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_DHT,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_SOFn] = {
        .ReadSegment = NULL,
        .SetSegmentMap = NULL,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_SOFn,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_DRI] = {
        .ReadSegment = NULL,
        .SetSegmentMap = NULL,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_DRI,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_SOS] = {
        .ReadSegment = ReadJpegSegment_SOS,
        .SetSegmentMap = SetSegmentMap_SOS,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_SOS,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_RSTn] = {
        .ReadSegment = ReadJpegSegment_RSTn,
        .SetSegmentMap = SetSegmentMap_RSTn,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_RSTn,
    },
    [PICTURE_JPEG_SEGMENT_TYPE_EOI] = {
        .ReadSegment = ReadJpegSegment_EOI,
        .SetSegmentMap = SetSegmentMap_SOI_EOI,
        .UpdateSegmentInfoWindow = UpdateSegmentInfoWindow_EOI,
    },
};
static const HANPCSTR sg_pDHTTableType[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_CNT] = {
    [PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC] = TEXT("DC表"),
    [PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC] = TEXT("AC表"),
};
static const uint8_t sg_pComponentIdTable[6] = {
    [0] = 255,
    [1] = 0,
    [2] = 1,
    [3] = 2,
    [4] = 1,
    [5] = 2,
};
static const HANSIZE sg_pRestorIdTable[2][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, },
    { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, },
};
static const uint8_t sg_pZigZagOrderTable[8][8] = {
    { 0,  1,  5,  6,  14, 15, 27, 28, },
    { 2,  4,  7,  13, 16, 26, 29, 42, },
    { 3,  8,  12, 17, 25, 30, 41, 43, },
    { 9,  11, 18, 24, 31, 40, 44, 53, },
    { 10, 19, 23, 32, 39, 45, 52, 54, },
    { 20, 22, 33, 38, 46, 51, 55, 60, },
    { 21, 34, 37, 47, 50, 56, 59, 61, },
    { 35, 36, 48, 49, 57, 58, 62, 63, },
};
static const PICTUREJPEGDECODEMODEINFO sg_pDecodeProcedureMode[PICTURE_JPEG_DECODE_PROCEDURE_MODE_CNT] = {
    [PICTURE_JPEG_DECODE_PROCEDURE_MODE_AUTO] = {
        .pText = TEXT("自动"),
        .DecodeCallback = JpegDecodeModeAuto,
    },
    [PICTURE_JPEG_DECODE_PROCEDURE_MODE_SLOW] = {
        .pText = TEXT("精度优先"),
        .DecodeCallback = JpegDecodeModeSlow,
    },
    [PICTURE_JPEG_DECODE_PROCEDURE_MODE_FASE] = {
        .pText = TEXT("速度优先"),
        .DecodeCallback = JpegDecodeModeFast,
    },
};

BOOL CheckJpegType(const uint8_t* pData, HANSIZE nLen)
{
    BOOL bRet;

    if (nLen < sizeof(sg_pJpegHeader)) { bRet = FALSE; }
    else if (0 != memcmp(pData, sg_pJpegHeader, sizeof(sg_pJpegHeader))) { bRet = FALSE; }
    else { bRet = TRUE; }

    return bRet;
}

void RegisterHANPictureJpeg(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureJpegWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREJPEGWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)GetStockObject(PICTURE_BACKGROUND_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_JPEG_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);

    INITCOMMONCONTROLSEX icexInitToolTip;
    icexInitToolTip.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icexInitToolTip.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icexInitToolTip);
    
    RegisterHANPictureJpegMCU(hInst);
    RegisterHANPictureJpegPaint(hInst);
}

void SavePictureJpeg(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam)
{
    uint8_t pEOI[2] = { 0xFF, 0xD9, };
    PPICTUREJPEGWNDEXTRA jpegInfo = (PPICTUREJPEGWNDEXTRA)(pSaveParam->pParam);
    PICTUREJPEGSAVEPARAM jpegSaveParam = {
        .jpegInfo = jpegInfo,
        .pPicture = pSaveParam->pPicture,
        .nComponent = 3,
    };
    HANDLE hFile;
    PPICTUREJPEGENCODEMACHINE pEncode;

    if (TRUE == CreateJpegSaveParamDialog(&jpegSaveParam))
    {
        pEncode = AllocJpegEncodeMemory(&jpegSaveParam);
        if (NULL != pEncode)
        {
            hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

            if (INVALID_HANDLE_VALUE != hFile)
            {
                WriteFile(hFile, sg_pJpegHeader, sizeof(sg_pJpegHeader), NULL, NULL);
                SaveJpegWriteDQT(pEncode, hFile);
                SaveJpegWriteSOFn(pEncode, hFile);
                SaveJpegWriteDHT(pEncode, hFile);
                SaveJpegWriteSOS(pEncode, hFile);

                WriteFile(hFile, pEOI, sizeof(pEOI), NULL, NULL);
                CloseHandle(hFile);
            }
            else { MessageBox(jpegInfo->hSelf, TEXT("打开文件失败"), TEXT("无法保存"), 0); }

            HANWinHeapFree(jpegInfo->hHeap, 0, pEncode);
        }
    }
}

static LRESULT CALLBACK PictureJpegWndProc(HWND hPictureJpeg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREJPEGWNDEXTRA jpegInfo = (PPICTUREJPEGWNDEXTRA)GetWindowLongPtr(hPictureJpeg, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPictureJpeg, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(hPictureJpeg, jpegInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)StaticColorCallback(jpegInfo, (HWND)lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(jpegInfo, wParam);
        } break;
        case WM_NOTIFY: {
            if (FALSE == NotifyCallback(jpegInfo, (NMHDR*)lParam)) { lWndProcRet = DefWindowProc(hPictureJpeg, message, wParam, lParam); }
        } break;
        case WM_DESTROY: {
            DestroyCallback(jpegInfo);
            lWndProcRet = DefWindowProc(hPictureJpeg, message, wParam, lParam);
        } break;

        case PCTM_GETSAVEPARAM: {
            lWndProcRet = GetSaveParamCallback(jpegInfo, (PPICTURESAVEPARAM)lParam);
        } break;
        case PCTM_ZOOM: {
            (void)SendMessage(jpegInfo->hEditTool, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureJpeg, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPictureJpeg, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREJPEGWNDEXTRA jpegInfo;
    PPICTUREJPEGWNDEXTRA jpegTempInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PPICTURECREATEPARAM pPictureCreateParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    HANINT nSegmentCnt;

    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_WINDOW_DY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        jpegInfo = (PPICTUREJPEGWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREJPEGWNDEXTRA));
        if (NULL == jpegInfo) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureJpeg, 0, (LONG_PTR)jpegInfo);

        *jpegInfo = sg_jpegDefaultInfo;

        jpegInfo->hHeap = hHeap;
        jpegInfo->hInst = hInst;
        jpegInfo->hSelf = hPictureJpeg;
        jpegInfo->hBackground = CreateSolidBrush(PICTURE_JPEG_BACKGROUND_COLOR);
        memset(&(jpegInfo->segment.segmentInfo), 0, sizeof(jpegInfo->segment.segmentInfo));

        GetClientRect(hPictureJpeg, &rcClientSize);

        jpegInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);
        jpegInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        jpegInfo->hFont.hInfo = CreateFontIndirect(&g_lfInfoFont);

        nWinW = 80;
        nWinH = PICTURE_BUTTON_HEIGHT;
        jpegInfo->mcuView.hButton = CreateWindow(TEXT("button"), TEXT("查看MCU"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            nWinX, nWinY, nWinW, nWinH,
            hPictureJpeg, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_BUTTON, hInst, NULL);
        jpegInfo->mcuView.hWindow = NULL;
        
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinY = PICTURE_WINDOW_DY;
        nWinW = GetSegmentListWindowWidth();
        nWinH = PICTURE_JPEG_INFO_HEIGHT;
        jpegInfo->segment.hList = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPictureJpeg, (HMENU)WID_PICTURE_JPEG_SEGMENT_LIST, hInst, NULL);
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinW = GetSegmentInfoWindowWidth();
        jpegInfo->segment.hInfo = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPictureJpeg, (HMENU)WID_PICTURE_JPEG_SEGMENT_INFO, hInst, NULL);

        SendMessage(jpegInfo->mcuView.hButton, WM_SETFONT, (WPARAM)(jpegInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegInfo->segment.hList, WM_SETFONT, (WPARAM)(jpegInfo->hFont.hHex), (LPARAM)TRUE);
        SendMessage(jpegInfo->segment.hInfo, WM_SETFONT, (WPARAM)(jpegInfo->hFont.hInfo), (LPARAM)TRUE);

        ListView_SetExtendedListViewStyle(jpegInfo->segment.hList, LVS_EX_FULLROWSELECT);
        ListView_SetExtendedListViewStyle(jpegInfo->segment.hInfo, LVS_EX_FULLROWSELECT);
        
        InitJpegSegmentWindow(jpegInfo);
    }
    /* 解码文件的数据段，填充信息 */
    if (-1 != lWndProcRet)
    {
        jpegInfo->paramPicture = *pPictureCreateParam;
        nSegmentCnt = JpegProcess(jpegInfo);
        if (nSegmentCnt < 0) { lWndProcRet = -1; }
    }
    /* 重新分配 jpegInfo 内存 */
    if (-1 != lWndProcRet)
    {
        jpegInfo->segment.map.nCnt = nSegmentCnt;
        jpegTempInfo = ReallocJpegInfoMemory(jpegInfo);
        if (NULL != jpegTempInfo)
        {
            jpegInfo = jpegTempInfo;
            SetWindowLongPtr(hPictureJpeg, 0, (LONG_PTR)jpegInfo);
        }
        else { lWndProcRet = -1; }
    }
    // /* 图片解码 */
    if (-1 != lWndProcRet)
    {
        BOOL bDecodeRet = TRUE;

        SetSegmentMap(pPictureCreateParam->pData, nSegmentCnt, jpegInfo);
        bDecodeRet = DecodeSOS(jpegInfo);

        if (FALSE == bDecodeRet) { CreateWarningWindow(jpegInfo); }
        
        /* 绘制预览图 */
        nWinX += nWinW + PICTURE_WINDOW_DX;
        GetJpegShowSize(jpegInfo, &nWinW, &nWinH);
        jpegInfo->hShow = CreateWindow(HAN_PICTURE_JPEG_PAINT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER, nWinX, nWinY, nWinW, nWinH,
            hPictureJpeg, (HMENU)WID_PICTURE_JPEG_SHOW, hInst, jpegInfo->pictureData.pPictureInfo->pPicture[0]
        );
        nWinX = PICTURE_WINDOW_DX;
        nWinY += PICTURE_JPEG_INFO_HEIGHT + PICTURE_WINDOW_DY;
        nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
        nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;
        jpegInfo->hEditTool = CreateWindow(HAN_PICTURE_EDIT_TOOL_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, nWinW, nWinH,
            hPictureJpeg, (HMENU)WID_PICTURE_EDIT_TOOL, hInst, jpegInfo->pictureData.pPictureInfo
        );
        if ((NULL == jpegInfo->hShow) || (NULL == jpegInfo->hEditTool)) { lWndProcRet = -1; }
    }

    return lWndProcRet;
}
static void SizeCallback(HWND hPictureJpeg, PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_JPEG_INFO_HEIGHT + (PICTURE_WINDOW_DY * 2);
    HANINT nWinW;
    HANINT nWinH;
    RECT rcClientSize;
    
    GetClientRect(hPictureJpeg, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
    nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;

    MoveWindow(jpegInfo->hEditTool, nWinX, nWinY, nWinW, nWinH, TRUE);
}
static HBRUSH StaticColorCallback(PPICTUREJPEGWNDEXTRA jpegInfo, HWND hStatic)
{
    HBRUSH hRet;

    if (hStatic == jpegInfo->warning.hStatic) { hRet = jpegInfo->hBackground; }
    else { hRet = GetStockObject(WHITE_BRUSH); }

    return hRet;
}
static void CommandCallback(PPICTUREJPEGWNDEXTRA jpegInfo, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_PICTURE_JPEG_MCU_VIEW_BUTTON: {
            MCUViewCallback(jpegInfo);
        } break;

        default: { } break;
    }
}
static BOOL NotifyCallback(PPICTUREJPEGWNDEXTRA jpegInfo, NMHDR* pNotify)
{
    BOOL bRet = FALSE;

    switch (pNotify->idFrom) {
        case WID_PICTURE_JPEG_SEGMENT_LIST: {
            bRet = SegmentListNotifyCallback(jpegInfo, pNotify);
        } break;

        default: { } break;
    }

    return bRet;
}
static void DestroyCallback(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    if (NULL != jpegInfo->mcuView.hWindow) { DestroyWindow(jpegInfo->mcuView.hWindow); }
    HANWinHeapFree(jpegInfo->hHeap, 0, jpegInfo);
}
static LRESULT GetSaveParamCallback(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTURESAVEPARAM pSaveParam)
{
    HAN_strcpy(pSaveParam->pOpenClassName, HAN_PICTURE_JPEG_CLASS);
    pSaveParam->pPicture = jpegInfo->pictureData.pPictureInfo->pPicture[0];
    pSaveParam->pParam = jpegInfo;

    return TRUE;
}
static void InitJpegSegmentWindow(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };
    
    for (PICTUREJPEGSEGMENTLISTHEADER iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_LIST_HEADER_CNT; iLoop++)
    {
        lvTitle.iSubItem = iLoop;
        lvTitle.pszText = (HANPSTR)sg_pJpegSegmentListHeader[iLoop].pName;
        lvTitle.cx = sg_pJpegSegmentListHeader[iLoop].nWidth;
        lvTitle.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(jpegInfo->segment.hList, iLoop, &lvTitle);
    }
    for (PICTUREJPEGSEGMENTINFOHEADER iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_INFO_HEADER_CNT; iLoop++)
    {
        lvTitle.iSubItem = iLoop;
        lvTitle.pszText = TEXT("");
        lvTitle.cx = sg_pJpegSegmentInfoHeaderWidth[iLoop];
        lvTitle.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(jpegInfo->segment.hInfo, iLoop, &lvTitle);
    }
}
static HANINT JpegProcess(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANINT nRet = 0;
    const uint8_t* pData = jpegInfo->paramPicture.pData;
    HANSIZE nLen = jpegInfo->paramPicture.nLen;
    HWND hListViewReport = jpegInfo->segment.hList;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    HANSIZE nOffset = 0;
    PICTUREJPEGSEGMENT jpegSegment;
    HANSIZE nSegmetLen;
    LVITEM lvItem = { .mask = LVIF_TEXT, };

    while (nOffset < nLen)
    {
        nSegmetLen = ReadJpegSegment(&pData[nOffset], nLen - nOffset, &jpegSegment);
        if (0 < nSegmetLen)
        {
            lvItem.iItem = nRet;
            lvItem.iSubItem = 0;
            lvItem.pszText = pText;
            ListView_InsertItem(hListViewReport, &lvItem);

            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_LIST_HEADER_MARKER;
            HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), jpegSegment.cMarker);
            ListView_SetItem(hListViewReport, &lvItem);
            
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_LIST_HEADER_LEN;
            HAN_snprintf(pText, ArrLen(pText), TEXT("%u"), jpegSegment.nLength);
            ListView_SetItem(hListViewReport, &lvItem);
            
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_LIST_HEADER_EXLEN;
            HAN_snprintf(pText, ArrLen(pText), TEXT(HANSIZE_PRINT_FORMAT), jpegSegment.nExLength);
            ListView_SetItem(hListViewReport, &lvItem);
            
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_LIST_HEADER_DATA;
            PictureJpegPrintHexData(pText, ArrLen(pText), jpegSegment.pData, jpegSegment.nLength + jpegSegment.nExLength);
            ListView_SetItem(hListViewReport, &lvItem);
            
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_LIST_HEADER_ADDRESS;
            HAN_snprintf(pText, ArrLen(pText), TEXT(HANSIZE_PRINT_HEX8_FORMAT), (uint32_t)nOffset);
            ListView_SetItem(hListViewReport, &lvItem);

            nRet++;
            nOffset += nSegmetLen;

            if (0xFFD9 == jpegSegment.cMarker) { nOffset = nLen; }
            else { (void)UpdateSegmentInfo(&jpegSegment, &(jpegInfo->segment.segmentInfo)); }
        }
        else
        {
            nRet = 0;
            nOffset = nLen;
        }
    }
    
    return nRet;
}
static PPICTUREJPEGWNDEXTRA ReallocJpegInfoMemory(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PPICTUREJPEGWNDEXTRA pRet;
    PICTURERESOLUTION pxResolution = jpegInfo->segment.segmentInfo.SOFn.pxResolution;
    PICTURERESOLUTION pxMCUSize = jpegInfo->segment.segmentInfo.SOFn.pxMCUSize;
    PICTURERESOLUTION pxMCUCnt = jpegInfo->segment.segmentInfo.SOFn.pxMCUCnt;
    HANSIZE nSegmentCnt = jpegInfo->segment.map.nCnt;
    SIZE_T nOffset;
    SIZE_T nSegmentSize;
    SIZE_T nBlockCnt = jpegInfo->segment.segmentInfo.SOFn.nBlockTotal;
    SIZE_T nDecoded1DSize;
    SIZE_T nDecoded2DSize;
    SIZE_T nIDCTSize;
    SIZE_T nYCbCrSize;
    SIZE_T nPictureInfoSize;
    SIZE_T nPictureSize;
    SIZE_T nMCUInfoSize;

    nSegmentSize = sizeof(PICTUREJPEGSEGMENT) * nSegmentCnt;
    nDecoded1DSize = sizeof(PICTUREJPEGDCTMATRIX1D) * nBlockCnt;
    nDecoded2DSize = sizeof(PICTUREJPEGDCTMATRIX2D) * nBlockCnt;
    nIDCTSize = sizeof(PICTUREJPEGDCTMATRIX2D) * nBlockCnt;
    nYCbCrSize = sizeof(PICTUREJPEGYCBCR) * (pxMCUSize.pxWidth * pxMCUCnt.pxWidth) * (pxMCUSize.pxHeight * pxMCUCnt.pxHeight);
    nPictureInfoSize = GetPictureInfoMemSize(1);
    nPictureSize = GetPictureMemSize(&pxResolution);
    nMCUInfoSize = GetMCUInfoSize(jpegInfo);
    pRet = (PPICTUREJPEGWNDEXTRA)HANWinHeapAlloc(
        jpegInfo->hHeap, jpegInfo,
        sizeof(PICTUREJPEGWNDEXTRA)
         + nSegmentSize
         + nDecoded1DSize
         + nDecoded2DSize
         + nIDCTSize
         + nYCbCrSize
         + nPictureInfoSize
         + nPictureSize
         + nMCUInfoSize
    );

    if (NULL != pRet)
    {
        nOffset = 0;
        pRet->segment.map.pSegmentList = (PPICTUREJPEGSEGMENT)&(pRet->pBuf[nOffset]); nOffset += nSegmentSize;
        pRet->pictureData.nDecodedDataLen = nBlockCnt;
        pRet->pictureData.pDecodedData1D = (PICTUREJPEGDCTMATRIX1D*)&(pRet->pBuf[nOffset]); nOffset += nDecoded1DSize;
        pRet->pictureData.pDecodedData2D = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nDecoded2DSize;
        pRet->pictureData.nIDCTDataLen = nBlockCnt;
        pRet->pictureData.pIDCTData = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nIDCTSize;
        pRet->pictureData.nYCbCrDataLen = (pxMCUSize.pxWidth * pxMCUCnt.pxWidth) * (pxMCUSize.pxHeight * pxMCUCnt.pxHeight);
        pRet->pictureData.pYCbCrData = (PICTUREJPEGYCBCR*)&(pRet->pBuf[nOffset]); nOffset += nYCbCrSize;
        pRet->pictureData.pPictureInfo = (HANPPICTUREINFO)&(pRet->pBuf[nOffset]); nOffset += nPictureInfoSize;
        pRet->pictureData.pPictureInfo->nCnt = 1;
        pRet->pictureData.pPictureInfo->pPicture[0] = (HANPPICTURE)&(pRet->pBuf[nOffset]); nOffset += nPictureSize;
        InitMCUInfoMap(pRet, &(pRet->pBuf[nOffset])); nOffset += nMCUInfoSize;
        UpdatePictureMap(pRet->pictureData.pPictureInfo->pPicture[0], &pxResolution);
    }

    return pRet;
}
static HANSIZE GetMCUInfoSize(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANSIZE nRet = 0;
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = &(jpegInfo->segment.segmentInfo);
    PPICTURERESOLUTION pMCUCnt = &(pSegmentInfo->SOFn.pxMCUCnt);
    HANSIZE nMCUTotal = pMCUCnt->pxWidth * pMCUCnt->pxHeight;
    HANSIZE nSOSTotal;
    uint8_t nMCUComponentBlockCnt;

    for (HANSIZE iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_COMPONENT_MAX; iLoop++)
    {
        nMCUComponentBlockCnt = pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop];
        nSOSTotal = pSegmentInfo->SOS.pSOSTotal[iLoop];
        if (0 < nMCUComponentBlockCnt)
        {
            nRet += (sizeof(PICTUREJPEGMCUINFO) + (sizeof(PICTUREJPEGMCUPOS) * ((nMCUTotal * nMCUComponentBlockCnt) + 1))) * nSOSTotal;
        }
    }

    return nRet;
}
static void InitMCUInfoMap(PPICTUREJPEGWNDEXTRA jpegInfo, uint8_t* pBuf)
{
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = &(jpegInfo->segment.segmentInfo);
    PPICTURERESOLUTION pMCUCnt = &(pSegmentInfo->SOFn.pxMCUCnt);
    PPICTUREJPEGMCUINFO pMCUInfoTable = (PPICTUREJPEGMCUINFO)pBuf;
    uint8_t* pBlockPos = pBuf;
    HANSIZE nMCUTotal = pMCUCnt->pxWidth * pMCUCnt->pxHeight;
    HANSIZE nSOSTotal;
    HANSIZE nMCUId;
    uint8_t nMCUComponentBlockCnt;
    /* 先跳过信息表，找好第一个 MCU 缓存的位置 */
    for (HANSIZE iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_COMPONENT_MAX; iLoop++)
    {
        nMCUComponentBlockCnt = pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop];
        if (0 < nMCUComponentBlockCnt)
        {
            nSOSTotal = pSegmentInfo->SOS.pSOSTotal[iLoop];
            pBlockPos = &pBlockPos[sizeof(PICTUREJPEGMCUINFO) * nSOSTotal];
        }
    }
    /* 设置信息表 */
    nMCUId = 0;
    for (HANSIZE iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_COMPONENT_MAX; iLoop++)
    {
        nMCUComponentBlockCnt = pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop];
        nSOSTotal = pSegmentInfo->SOS.pSOSTotal[iLoop];
        if (0 < nMCUComponentBlockCnt)
        {
            jpegInfo->pictureData.mcuTable[iLoop].pInfo = &pMCUInfoTable[nMCUId];
            jpegInfo->pictureData.mcuTable[iLoop].nCnt = nSOSTotal;
            for (HANSIZE jLoop = 0; jLoop < nSOSTotal; jLoop++)
            {
                pMCUInfoTable[nMCUId].bValid = FALSE; // 初始化为无效
                pMCUInfoTable[nMCUId].pBlockPos = (PPICTUREJPEGBLOCKPOS)pBlockPos;
                pBlockPos = &pBlockPos[sizeof(PICTUREJPEGMCUPOS) * ((nMCUTotal * nMCUComponentBlockCnt) + 1)];
                nMCUId++;
            }
        }
    }
}
static HANSIZE ReadJpegSegment(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment)
{
    HANSIZE nRet = 0;
    HANSIZE nSegmentLength;
    PICTUREJPEGSEGMENT jpegSegment;

    for (PICTUREJPEGSEGMENTTYPE iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_TYPE_CNT; iLoop++)
    {
        if (NULL != sg_pJpegSegmentType[iLoop].ReadSegment)
        {
            nSegmentLength = sg_pJpegSegmentType[iLoop].ReadSegment(pData, nLen, &jpegSegment);
            if (0 < nSegmentLength)
            {
                *pSegment = jpegSegment;
                nRet = nSegmentLength;
            }
        }
    }
    if (0 == nRet)
    {
        nSegmentLength = ReadJpegSegment_Default(pData, nLen, &jpegSegment);
        if (0 < nSegmentLength)
        {
            *pSegment = jpegSegment;
            nRet = nSegmentLength;
        }
    }

    return nRet;
}
static HANSIZE GetNextSegmentPos(const uint8_t* pData, HANSIZE nLen)
{
    HANSIZE nRet = nLen;

    if (2 <= nLen)
    {
        for (HANSIZE iLoop = 0; iLoop < nLen - 1; iLoop++)
        {
            if ((0xFF == pData[iLoop]) && (0x00 != pData[iLoop + 1]))
            {
                nRet = iLoop;
                break;
            }
        }
    }

    return nRet;
}
static void GetJpegShowSize(PPICTUREJPEGWNDEXTRA jpegInfo, HANINT* pW, HANINT* pH)
{
    HANDOUBLE pxWidth = (HANDOUBLE)(jpegInfo->segment.segmentInfo.SOFn.pxResolution.pxWidth);
    HANDOUBLE pxHeight = (HANDOUBLE)(jpegInfo->segment.segmentInfo.SOFn.pxResolution.pxHeight);
    
    *pH = PICTURE_JPEG_INFO_HEIGHT;
    *pW = (HANINT)((HANDOUBLE)(*pH) / pxHeight * pxWidth);
}
static void SetSegmentMap(const uint8_t* pData, HANINT nSegmentCnt, PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANCHAR pAddr[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    HANSIZE nOffset = 0;
    BOOL bSetMapOk;
    PPICTUREJPEGSEGMENT pSegmentList = jpegInfo->segment.map.pSegmentList;
    PPICTUREJPEGSEGMENT pLastSOS = NULL;

    for (HANINT iLoop = 0; iLoop < nSegmentCnt; iLoop++)
    {
        ListView_GetItemText(jpegInfo->segment.hList, iLoop, PICTURE_JPEG_SEGMENT_LIST_HEADER_ADDRESS, pAddr, ArrLen(pAddr));
        nOffset = HAN_strtoul(pAddr, NULL, 16);
        pSegmentList[iLoop].cMarker = ReadJpegData2ByteMSB(&pData[nOffset + 0]);
        pSegmentList[iLoop].pSegment = &pData[nOffset + 0];
        pSegmentList[iLoop].nPos = nOffset;

        for (HANSIZE jLoop = 0; jLoop < PICTURE_JPEG_SEGMENT_TYPE_CNT; jLoop++)
        {
            bSetMapOk = FALSE;
            if (NULL != sg_pJpegSegmentType[jLoop].SetSegmentMap)
            {
                if (TRUE == sg_pJpegSegmentType[jLoop].SetSegmentMap(&pData[nOffset], &pLastSOS, &pSegmentList[iLoop]))
                {
                    bSetMapOk = TRUE;
                    break;
                }
            }
        }
        if (FALSE == bSetMapOk)
        {
            SetSegmentMap_Default(&pData[nOffset], &pLastSOS, &pSegmentList[iLoop]);
        }
    }
}
static void CreateWarningWindow(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HICON hIcon;
    HANINT nSize = 32;
    HANINT nWinX= PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_JPEG_INFO_HEIGHT + PICTURE_WINDOW_DY - nSize;
    TOOLINFO toolInfo = { 0 };
    BOOL nAddToolRet;
    
    jpegInfo->warning.hStatic = CreateWindow(
        TEXT("static"), NULL,
        WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY, nWinX, nWinY, nSize, nSize,
        jpegInfo->hSelf, (HMENU)WID_PICTURE_JPEG_WARNING_ICON, jpegInfo->hInst, NULL
    );
    jpegInfo->warning.hToolTip = CreateWindowEx(
        WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        jpegInfo->hSelf, NULL, jpegInfo->hInst, NULL
    );

    hIcon = LoadIcon(NULL, IDI_WARNING);
    SendMessage(jpegInfo->warning.hStatic, STM_SETICON, (WPARAM)hIcon, 0);

    SetWindowPos(jpegInfo->warning.hToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    toolInfo.cbSize = sizeof(TOOLINFO);
    toolInfo.hwnd = jpegInfo->hSelf;
    toolInfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    toolInfo.lpszText = TEXT("图片解码有错误，仅显示解码成功的部分");
    toolInfo.uId = (UINT_PTR)(jpegInfo->warning.hStatic);
    nAddToolRet = (BOOL)SendMessage(jpegInfo->warning.hToolTip, TTM_ADDTOOL, 0, (LPARAM)(&toolInfo));
    if (FALSE == nAddToolRet) // TTM_ADDTOOL 由于版本问题，可能会因 cbSize 成员大小导致不兼容问题，如果添加失败，尝试用低版本大小添加
    {
        toolInfo.cbSize -= sizeof(void*);
        SendMessage(jpegInfo->warning.hToolTip, TTM_ADDTOOL, 0, (LPARAM)(&toolInfo));
    }
    SendMessage(jpegInfo->warning.hToolTip, TTM_ACTIVATE, (WPARAM)TRUE, 0);
}
static void MCUViewCallback(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    HANSIZE nTextLen;
    PICTUREJPEGMCUCREATEPARAM mcuParam = {
        .pWindow = &(jpegInfo->mcuView.hWindow),
        .pPicture = jpegInfo->pictureData.pPictureInfo->pPicture[0],
        .pSegmentInfo = &(jpegInfo->segment.segmentInfo),
        .pComponent = jpegInfo->pictureData.pIDCTData,
        .pDCT = jpegInfo->pictureData.pDecodedData2D,
        .pMCUTable = jpegInfo->pictureData.mcuTable,
    };

    if (NULL == jpegInfo->mcuView.hWindow)
    {
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("MCU查看器 - "));
        pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
        nTextLen = HAN_strlen(pText);
        GetWindowText(jpegInfo->hSelf, &pText[nTextLen], (HANINT)(HAN_PICTURE_JPEG_TEXT_BUF_SIZE - nTextLen));
        jpegInfo->mcuView.hWindow = CreateWindow(HAN_PICTURE_JPEG_MCU_CLASS, pText,
            WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, 0, 500, 500,
            NULL, NULL, jpegInfo->hInst, &mcuParam);
    }
    else
    {
        ShowWindow(jpegInfo->mcuView.hWindow, SW_RESTORE);
        SetForegroundWindow(jpegInfo->mcuView.hWindow);
    }
}
static BOOL SegmentListNotifyCallback(PPICTUREJPEGWNDEXTRA jpegInfo, NMHDR* pNotify)
{
    BOOL bRet = FALSE;

    switch (pNotify->code) {
        case NM_CLICK: {
            NMITEMACTIVATE* pItemAct = (NMITEMACTIVATE*)pNotify;
            if (-1 != pItemAct->iItem)
            {
                UpdateSegmentInfoWindow(jpegInfo, pItemAct->iItem);
                bRet = TRUE;
            }
        } break;

        default: { } break;
    }

    return bRet;
}
static BOOL UpdateSegmentInfo(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bOk = TRUE;

    switch (pSegment->cMarker) {
        case 0xFFDB: { bOk = UpdateSegmentInfo_DQT(pSegment, pSegmentInfo); } break;
        case 0xFFC4: { bOk = UpdateSegmentInfo_DHT(pSegment, pSegmentInfo); } break;
        case 0xFFC0: { bOk = UpdateSegmentInfo_SOFn(pSegment, pSegmentInfo); } break;
        case 0xFFC2: { bOk = UpdateSegmentInfo_SOFn(pSegment, pSegmentInfo); } break;
        case 0xFFDA: { bOk = UpdateSegmentInfo_SOS(pSegment, pSegmentInfo); } break;

        default: { } break;
    }

    return bOk;
}
static void UpdateSegmentInfoWindow(PPICTUREJPEGWNDEXTRA jpegInfo, HANINT nId)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    BOOL bOk = FALSE;
    PICTUREJPEGSEGMENT jpegSegment;
    HANSIZE nAddr;

    SendMessage(jpegInfo->segment.hInfo, WM_SETREDRAW, FALSE, 0);

    ListView_DeleteAllItems(jpegInfo->segment.hInfo);
    ListView_GetItemText(jpegInfo->segment.hList, nId, PICTURE_JPEG_SEGMENT_LIST_HEADER_MARKER, pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    jpegSegment.cMarker = (uint16_t)HAN_strtoul(pText, NULL, 16);
    ListView_GetItemText(jpegInfo->segment.hList, nId, PICTURE_JPEG_SEGMENT_LIST_HEADER_LEN, pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    jpegSegment.nLength = (uint16_t)HAN_strtoul(pText, NULL, 10);
    ListView_GetItemText(jpegInfo->segment.hList, nId, PICTURE_JPEG_SEGMENT_LIST_HEADER_EXLEN, pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    jpegSegment.nExLength = HAN_strtoul(pText, NULL, 10);
    ListView_GetItemText(jpegInfo->segment.hList, nId, PICTURE_JPEG_SEGMENT_LIST_HEADER_ADDRESS, pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nAddr = HAN_strtoul(pText, NULL, 16);
    jpegSegment.pSegment = &(jpegInfo->paramPicture.pData[nAddr]);
    jpegSegment.pData = &(jpegInfo->paramPicture.pData[nAddr + 4]);
    for (PICTUREJPEGSEGMENTTYPE iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_TYPE_CNT; iLoop++)
    {
        if (NULL != sg_pJpegSegmentType[iLoop].UpdateSegmentInfoWindow)
        {
            if (TRUE == sg_pJpegSegmentType[iLoop].UpdateSegmentInfoWindow(&jpegSegment, jpegInfo->segment.hInfo))
            {
                bOk = TRUE;
                break;
            }
        }
    }
    if (FALSE == bOk) { UpdateSegmentInfoWindow_Default(&jpegSegment, jpegInfo->segment.hInfo); }

    SendMessage(jpegInfo->segment.hInfo, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(jpegInfo->segment.hInfo, NULL, TRUE);
}
static void PrintHuffmanCode(PCPICTUREJPEGHUFFMAN pHuffman, HANPSTR pText, size_t nLen)
{
    uint8_t nCodeLen = pHuffman->nLen;
    uint16_t cCode = pHuffman->cCode;

    if (0 < nCodeLen)
    {
        for (size_t iLoop = 0; iLoop < nCodeLen; iLoop++)
        {
            HAN_snprintf(&pText[iLoop], nLen - iLoop, TEXT("%u"), (cCode >> (nCodeLen - 1 - iLoop)) & 1);
        }
    }
}

static BOOL CreateJpegSaveParamDialog(PPICTUREJPEGSAVEPARAM pParam)
{
    BOOL bRet = FALSE;
    HGLOBAL hData;
    uint8_t* pDialogInfo;
    DLGTEMPLATE* pDlgInfoTitle;
    WORD* pWord;
    WCHAR* pText;
    HANSIZE nOffset = 0;

    hData = GlobalAlloc(GMEM_ZEROINIT, 1024);
    if (NULL != hData)
    {
        pDialogInfo = GlobalLock(hData);
        if (NULL != pDialogInfo)
        {
            pDlgInfoTitle = (void*)(&pDialogInfo[nOffset]);
            pDlgInfoTitle->style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_BORDER | DS_MODALFRAME | WS_CAPTION;
            pDlgInfoTitle->cdit = 0;
            pDlgInfoTitle->x = 100;
            pDlgInfoTitle->y = 100;
            pDlgInfoTitle->cx = (PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH + PICTURE_JPEG_SAVE_PARAM_CHOOSE_BUTTON_WIDTH + (PICTURE_WINDOW_DX * 6)) / 2;
            pDlgInfoTitle->cy = ((PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT * 10) + (PICTURE_WINDOW_DY * 7)) / 2;
            nOffset += sizeof(DLGTEMPLATE);
            
            pWord = (void*)(&pDialogInfo[nOffset]);
            pWord[0] = 0; // 没有菜单
            pWord[1] = 0; // 没有类
            nOffset += sizeof(*pWord) * 2;
            pText = (void*)(&pDialogInfo[nOffset]);
            MultiByteToWideChar(GetACP(), 0, "JPEG保存选项", -1, pText, 50);

            GlobalUnlock(hData);
            bRet = (BOOL)DialogBoxIndirectParam(
                pParam->jpegInfo->hInst, (LPDLGTEMPLATE)hData, pParam->jpegInfo->hSelf,
                JpegSaveParamDialogProc, (LPARAM)pParam
            );
            GlobalFree(hData);
        }
    }

    return bRet;
}
static INT_PTR CALLBACK JpegSaveParamDialogProc(HWND hSaveParam, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nRet = TRUE;
    
    // 读取属性
    PPICTUREJPEGSAVEPARAM pSaveParam = (PPICTUREJPEGSAVEPARAM)GetWindowLongPtr(hSaveParam, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG: {
            JpegSaveParamInitCallback(hSaveParam, lParam);
        } break;
        case WM_COMMAND: {
            JpegSaveParamCommandCallback(hSaveParam, pSaveParam, wParam);
        } break;
        case WM_CLOSE: {
            EndDialog(hSaveParam, FALSE);
        } break;
        case WM_DESTROY: {
            EndDialog(hSaveParam, FALSE);
        } break;

        default: {
            nRet = FALSE;
        } break;
    }

    return nRet;
}
static void JpegSaveParamInitCallback(HWND hSaveParam, LPARAM lParam)
{
    PPICTUREJPEGSAVEPARAM pSaveParam = (PPICTUREJPEGSAVEPARAM)lParam;
    HINSTANCE hInst = pSaveParam->jpegInfo->hInst;
    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    SetWindowLongPtr(hSaveParam, DWLP_USER, (LONG_PTR)pSaveParam);

    nWinX = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH + (PICTURE_WINDOW_DX * 5);
    nWinY = PICTURE_WINDOW_DY;
    nWinW = PICTURE_JPEG_SAVE_PARAM_CHOOSE_BUTTON_WIDTH;
    nWinH = PICTURE_JPEG_SAVE_PARAM_CHOOSE_BUTTON_HEIGHT;
    pSaveParam->hOK = CreateWindow(TEXT("button"), TEXT("确定"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        hSaveParam, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_OK, hInst, NULL
    );
    nWinY += nWinH + PICTURE_WINDOW_DY;
    pSaveParam->hCancel = CreateWindow(TEXT("button"), TEXT("取消"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        hSaveParam, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_CANCEL, hInst, NULL
    );
    SendMessage(pSaveParam->hOK, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hCancel, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);

    nWinX = PICTURE_WINDOW_DX;
    nWinY = PICTURE_WINDOW_DY;
    nWinW = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH + (PICTURE_WINDOW_DX * 3);
    nWinH = (PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT * 4) + PICTURE_WINDOW_DY;
    pSaveParam->hSampleQuality.hTitle = CreateWindow(TEXT("button"), TEXT("采样质量"),
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        nWinX, nWinY, nWinW, nWinH,
        hSaveParam, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_TITLE, hInst, NULL
    );
    SetWindowSubclass(pSaveParam->hSampleQuality.hTitle, JpegSaveParamGroupBoxSubClassCallback, 0, (DWORD_PTR)pSaveParam);
    JpegSaveParamCreateSampleQualityWindows(pSaveParam);

    nWinY += nWinH + PICTURE_WINDOW_DY;
    nWinH = (PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT * 3) + PICTURE_WINDOW_DY;
    pSaveParam->hQuantizeQuality.hTitle = CreateWindow(TEXT("button"), TEXT("量化质量"),
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        nWinX, nWinY, nWinW, nWinH,
        hSaveParam, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_TITLE, hInst, NULL
    );
    SetWindowSubclass(pSaveParam->hQuantizeQuality.hTitle, JpegSaveParamGroupBoxSubClassCallback, 0, (DWORD_PTR)pSaveParam);
    JpegSaveParamCreateQuantizeQualityWindows(pSaveParam);

    nWinY += nWinH + PICTURE_WINDOW_DY;
    nWinH = (PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT * 3) + PICTURE_WINDOW_DY;
    pSaveParam->hType.hTitle = CreateWindow(TEXT("button"), TEXT("类型"),
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        nWinX, nWinY, nWinW, nWinH,
        hSaveParam, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_TYPE_TITLE, hInst, NULL
    );
    SetWindowSubclass(pSaveParam->hType.hTitle, JpegSaveParamGroupBoxSubClassCallback, 0, (DWORD_PTR)pSaveParam);
    JpegSaveParamCreateTypeWindows(pSaveParam);
}
static LRESULT JpegSaveParamGroupBoxSubClassCallback(HWND hGroupBox, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR nId, DWORD_PTR pParam)
{
    (void)nId;
    PPICTUREJPEGSAVEPARAM pSaveParam = (PPICTUREJPEGSAVEPARAM)pParam;

    switch (message) {
        case WM_COMMAND: {
            JpegSaveParamCommandCallback(hGroupBox, pSaveParam, wParam);
        } break;

        default: { } break;
    }

    return DefSubclassProc(hGroupBox, message, wParam, lParam);
}
static void JpegSaveParamCreateSampleQualityWindows(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    HINSTANCE hInst = pSaveParam->jpegInfo->hInst;
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    HANINT nWinW = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH;
    HANINT nWinH = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;

    nWinX = PICTURE_WINDOW_DX;
    nWinY = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    nWinW = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH;
    nWinH = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    pSaveParam->hSampleQuality.hLow = CreateWindow(TEXT("button"), TEXT("低（4:2:0）"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hSampleQuality.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_LOW, hInst, NULL
    );
    nWinY += PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    pSaveParam->hSampleQuality.hMiddle = CreateWindow(TEXT("button"), TEXT("中（4:2:1）"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hSampleQuality.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_MIDDLE, hInst, NULL
    );
    nWinY += PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    pSaveParam->hSampleQuality.hHigh = CreateWindow(TEXT("button"), TEXT("高（4:4:4）"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hSampleQuality.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_HIGH, hInst, NULL
    );
    SendMessage(pSaveParam->hSampleQuality.hTitle, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hSampleQuality.hLow, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hSampleQuality.hMiddle, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hSampleQuality.hHigh, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    ButtonSetChecked(pSaveParam->hSampleQuality.hLow);
    JpegSaveParamSetSamplingCoeLow(pSaveParam);
}
static void JpegSaveParamCreateQuantizeQualityWindows(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    HINSTANCE hInst = pSaveParam->jpegInfo->hInst;
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    HANINT nWinW = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH;
    HANINT nWinH = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;

    pSaveParam->hQuantizeQuality.hOfficial = CreateWindow(TEXT("button"), TEXT("低（官方）"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hQuantizeQuality.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_OFFICIAL, hInst, NULL
    );
    nWinY += PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    pSaveParam->hQuantizeQuality.hHigh = CreateWindow(TEXT("button"), TEXT("高"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hQuantizeQuality.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_HIGH, hInst, NULL
    );
    SendMessage(pSaveParam->hQuantizeQuality.hTitle, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hQuantizeQuality.hOfficial, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hQuantizeQuality.hHigh, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    ButtonSetChecked(pSaveParam->hQuantizeQuality.hHigh);
    pSaveParam->eQuantizationTable = PICTURE_SAVE_JPEG_QUANTIZATION_TABLE_HIGH;
}
static void JpegSaveParamCreateTypeWindows(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    HINSTANCE hInst = pSaveParam->jpegInfo->hInst;
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    HANINT nWinW = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH;
    HANINT nWinH = PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;

    pSaveParam->hType.hBaseline = CreateWindow(TEXT("button"), TEXT("基线"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hType.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_TYPE_BASELINE, hInst, NULL
    );
    nWinY += PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT;
    pSaveParam->hType.hProgressive = CreateWindow(TEXT("button"), TEXT("渐进"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        nWinX, nWinY, nWinW, nWinH,
        pSaveParam->hType.hTitle, (HMENU)WID_PICTURE_JPEG_SAVE_PARAM_TYPE_PROGRESSIVE, hInst, NULL
    );
    SendMessage(pSaveParam->hType.hTitle, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hType.hBaseline, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(pSaveParam->hType.hProgressive, WM_SETFONT, (WPARAM)(pSaveParam->jpegInfo->hFont.hSys), (LPARAM)TRUE);
    ButtonSetChecked(pSaveParam->hType.hBaseline);
    pSaveParam->jpegType = PICTURE_JPEG_TYPE_BASELINE;
}
static void JpegSaveParamCommandCallback(HWND hSaveParam, PPICTUREJPEGSAVEPARAM pSaveParam, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_PICTURE_JPEG_SAVE_PARAM_OK: {
            EndDialog(hSaveParam, TRUE);
        } break;
        case WID_PICTURE_JPEG_SAVE_PARAM_CANCEL: {
            EndDialog(hSaveParam, FALSE);
        } break;

        case WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_LOW: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hSampleQuality.hLow)) { JpegSaveParamSetSamplingCoeLow(pSaveParam); }
        } break;
        case WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_MIDDLE: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hSampleQuality.hMiddle)) { JpegSaveParamSetSamplingCoeMiddle(pSaveParam); }
        } break;
        case WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_HIGH: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hSampleQuality.hHigh)) { JpegSaveParamSetSamplingCoeHigh(pSaveParam); }
        } break;

        case WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_OFFICIAL: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hType.hBaseline)) { pSaveParam->eQuantizationTable = PICTURE_SAVE_JPEG_QUANTIZATION_TABLE_DEFAULT; }
        } break;
        case WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_HIGH: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hType.hBaseline)) { pSaveParam->eQuantizationTable = PICTURE_SAVE_JPEG_QUANTIZATION_TABLE_HIGH; }
        } break;

        case WID_PICTURE_JPEG_SAVE_PARAM_TYPE_BASELINE: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hType.hBaseline)) { pSaveParam->jpegType = PICTURE_JPEG_TYPE_BASELINE; }
        } break;
        case WID_PICTURE_JPEG_SAVE_PARAM_TYPE_PROGRESSIVE: {
            if (BST_CHECKED == ButtonGetCheck(pSaveParam->hType.hProgressive)) { pSaveParam->jpegType = PICTURE_JPEG_TYPE_PROGRESSIVE; }
        } break;

        default: { } break;
    }
}
static void JpegSaveParamSetSamplingCoeLow(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    pSaveParam->pSamplingCoe[0][0] = 2;
    pSaveParam->pSamplingCoe[0][1] = 2;
    pSaveParam->pSamplingCoe[1][0] = 1;
    pSaveParam->pSamplingCoe[1][1] = 1;
    pSaveParam->pSamplingCoe[2][0] = 1;
    pSaveParam->pSamplingCoe[2][1] = 1;
}
static void JpegSaveParamSetSamplingCoeMiddle(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    pSaveParam->pSamplingCoe[0][0] = 2;
    pSaveParam->pSamplingCoe[0][1] = 1;
    pSaveParam->pSamplingCoe[1][0] = 1;
    pSaveParam->pSamplingCoe[1][1] = 1;
    pSaveParam->pSamplingCoe[2][0] = 1;
    pSaveParam->pSamplingCoe[2][1] = 1;
}
static void JpegSaveParamSetSamplingCoeHigh(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    pSaveParam->pSamplingCoe[0][0] = 1;
    pSaveParam->pSamplingCoe[0][1] = 1;
    pSaveParam->pSamplingCoe[1][0] = 1;
    pSaveParam->pSamplingCoe[1][1] = 1;
    pSaveParam->pSamplingCoe[2][0] = 1;
    pSaveParam->pSamplingCoe[2][1] = 1;
}
static PPICTUREJPEGENCODEMACHINE AllocJpegEncodeMemory(PPICTUREJPEGSAVEPARAM pSaveParam)
{
    PPICTUREJPEGENCODEMACHINE pRet;
    HANPPICTURE pPicture = pSaveParam->pPicture;
    PPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    PICTURERESOLUTION pxMCUSize;
    PICTURERESOLUTION pxMCUCnt;
    PICTURERESOLUTION pxMCUBlockCnt;
    PICTURERESOLUTION pxSample;
    uint8_t pMCUComponentBlockCnt[3];
    uint8_t pMCUComponentBlockOffset[3];
    uint8_t nMCUComponentBlockTotal;
    uint8_t nBlockId;
    SIZE_T nMCUTotal;
    SIZE_T nBlockTotal;
    SIZE_T nYCbCrSize;
    SIZE_T nYCbCrMapSize;
    SIZE_T nComponentSize[3];
    SIZE_T nEncodeDataSize;
    SIZE_T nDataSize;
    SIZE_T nOffset;
    
    /* 默认亮度分量采样率最高，所以计算 MCU 和 Block 相关参数直接用亮度 */
    /* MCU 像素数 */
    pxMCUSize.pxWidth = pSaveParam->pSamplingCoe[0][0] * 8;
    pxMCUSize.pxHeight = pSaveParam->pSamplingCoe[0][1] * 8;
    /* MCU 横向和纵向的块数 */
    pxMCUBlockCnt.pxWidth = pSaveParam->pSamplingCoe[0][0];
    pxMCUBlockCnt.pxHeight = pSaveParam->pSamplingCoe[0][1];
    /* MCU 块数，向上取整 */
    pxMCUCnt.pxWidth = (pResolution->pxWidth + pxMCUSize.pxWidth - 1) / pxMCUSize.pxWidth;
    pxMCUCnt.pxHeight = (pResolution->pxHeight + pxMCUSize.pxHeight - 1) / pxMCUSize.pxHeight;
    nMCUTotal = pxMCUCnt.pxWidth * pxMCUCnt.pxHeight;
    /* 一个 MCU 内各个分量的块数 */
    nMCUComponentBlockTotal = 0;
    for (uint8_t iLoop = 0; iLoop < 3; iLoop++)
    {
        pMCUComponentBlockCnt[iLoop] = pSaveParam->pSamplingCoe[iLoop][0] * pSaveParam->pSamplingCoe[iLoop][1];
        pMCUComponentBlockOffset[iLoop] = nMCUComponentBlockTotal;
        nMCUComponentBlockTotal += pMCUComponentBlockCnt[iLoop];
    }
    /* 计算采样所需的图像尺寸 */
    pxSample.pxWidth = pxMCUSize.pxWidth * pxMCUCnt.pxWidth;
    pxSample.pxHeight = pxMCUSize.pxHeight * pxMCUCnt.pxHeight;
    /* 计算内存大小 */
    nYCbCrSize = sizeof(PICTUREJPEGYCBCR) * pxSample.pxWidth * pxSample.pxHeight;
    nYCbCrMapSize = sizeof(PPICTUREJPEGYCBCR) * pxSample.pxHeight;
    nComponentSize[0] = sizeof(PICTUREJPEGDCTMATRIX2D) * pMCUComponentBlockCnt[0] * nMCUTotal;
    nComponentSize[1] = sizeof(PICTUREJPEGDCTMATRIX2D) * pMCUComponentBlockCnt[1] * nMCUTotal;
    nComponentSize[2] = sizeof(PICTUREJPEGDCTMATRIX2D) * pMCUComponentBlockCnt[2] * nMCUTotal;
    nBlockTotal = nMCUComponentBlockTotal * nMCUTotal;
    nEncodeDataSize = sizeof(PICTUREJPEGDCTMATRIX1D) * nBlockTotal;
    nDataSize = PICTURE_JPEG_SAVE_ENCODE_BUF_SIZE;

    pRet = HANWinHeapAlloc(pSaveParam->jpegInfo->hHeap, NULL,
        sizeof(PICTUREJPEGENCODEMACHINE)
         + nYCbCrSize
         + nYCbCrMapSize
         + (nComponentSize[0] * 2)
         + (nComponentSize[1] * 2)
         + (nComponentSize[2] * 2)
         + nEncodeDataSize
         + nDataSize
    );

    if (NULL != pRet)
    {
        pRet->pPicture = pPicture;
        pRet->jpegType = pSaveParam->jpegType;
        pRet->nComponent = pSaveParam->nComponent;
        for (uint8_t iLoop = 0; iLoop < pRet->nComponent; iLoop++) { pRet->pSamplingCoe[iLoop] = pSaveParam->pSamplingCoe[iLoop]; }
        pRet->pxMCUCnt = pxMCUCnt;
        pRet->pxMCUBlockCnt = pxMCUBlockCnt;
        pRet->pxMCUSize = pxMCUSize;
        pRet->pxSample = pxSample;
        pRet->nMCUTotal = nMCUTotal;
        memcpy(pRet->pMCUComponentBlockCnt, pMCUComponentBlockCnt, sizeof(pRet->pMCUComponentBlockCnt[0]) * pRet->nComponent);
        memcpy(pRet->pMCUComponentBlockOffset, pMCUComponentBlockOffset, sizeof(pRet->pMCUComponentBlockOffset[0]) * pRet->nComponent);
        pRet->nMCUComponentBlockTotal = nMCUComponentBlockTotal;
        pRet->nBlockTotal = nBlockTotal;
        for (uint8_t iLoop = 0; iLoop < pRet->nComponent; iLoop++) { pRet->pComponentBlockTotal[iLoop] = pMCUComponentBlockCnt[iLoop] * nMCUTotal; }
        pRet->pQT = &g_pSaveJpegQuantizationTable[pSaveParam->eQuantizationTable];
        pRet->nQTId[0] = 0;
        pRet->nQTId[1] = 1;
        pRet->nQTId[2] = 1;
        nBlockId = 0;
        for (uint8_t iLoop = 0; iLoop < pRet->nComponent; iLoop++)
        {
            for (uint8_t jLoop = 0; jLoop < pMCUComponentBlockCnt[iLoop]; jLoop++)
            {
                pRet->pBlockComponentId[nBlockId] = iLoop;
                nBlockId++;
            }
        }

        nOffset = 0;
        pRet->pYCbCr = (PPICTUREJPEGYCBCR)&(pRet->pBuf[nOffset]); nOffset += nYCbCrSize;
        pRet->pYCbCrMap = (PPICTUREJPEGYCBCR*)&(pRet->pBuf[nOffset]); nOffset += nYCbCrMapSize;
        for (uint32_t iLoop = 0; iLoop < pxSample.pxHeight; iLoop++) { pRet->pYCbCrMap[iLoop] = &(pRet->pYCbCr[iLoop * pxSample.pxWidth]); }
        pRet->pComponentSample[0] = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nComponentSize[0];
        pRet->pComponentSample[1] = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nComponentSize[1];
        pRet->pComponentSample[2] = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nComponentSize[2];
        pRet->pComponentTransform[0] = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nComponentSize[0];
        pRet->pComponentTransform[1] = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nComponentSize[1];
        pRet->pComponentTransform[2] = (PICTUREJPEGDCTMATRIX2D*)&(pRet->pBuf[nOffset]); nOffset += nComponentSize[2];
        pRet->pEncodeData = (PICTUREJPEGDCTMATRIX1D*)&(pRet->pBuf[nOffset]); nOffset += nEncodeDataSize;
        pRet->pData = &(pRet->pBuf[nOffset]); nOffset += nDataSize;
    }

    return pRet;
}
static void SaveJpegWriteDQT(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    uint8_t pMarker[2] = { 0xFF, 0xDB };
    uint16_t nLen;
    uint8_t pQT[64];
    uint8_t* pData = pEncode->pData;
    HANSIZE nOffset = 0;

    memcpy(&pData[nOffset], pMarker, 2); nOffset += 4;

    for (uint8_t iLoop = 0; iLoop < pEncode->pQT->nCnt; iLoop++)
    {
        for (uint8_t nRow = 0; nRow < 8; nRow++)
        {
            for (uint8_t nCol = 0; nCol < 8; nCol++)
            {
                pQT[sg_pZigZagOrderTable[nRow][nCol]] = pEncode->pQT->pQuantization[iLoop][nRow][nCol];
            }
        }
        memcpy(&pData[nOffset], &iLoop, 1); nOffset += 1;
        memcpy(&pData[nOffset], pQT, 64); nOffset += 64;
    }

    nLen = (uint16_t)(nOffset - 2);
    WriteJpegData2ByteMSB(&pData[2], nLen);

    WriteFile(hFile, pData, (DWORD)nOffset, NULL, NULL);
}
static void SaveJpegWriteSOFn(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    PPICTURERESOLUTION pResolution = &(pEncode->pPicture->pxResolution);
    const uint8_t** pSamplingCoe = pEncode->pSamplingCoe;
    uint8_t pMarker[2] = { 0xFF, };
    uint16_t nLen = 17;
    uint8_t nSamplePrecision = 8;
    uint8_t pW[2] = { (uint8_t)(pResolution->pxWidth >> 8), (uint8_t)(pResolution->pxWidth), };
    uint8_t pH[2] = { (uint8_t)(pResolution->pxHeight >> 8), (uint8_t)(pResolution->pxHeight), };
    uint8_t nComponent = 3;
    uint8_t pComponentInfo[3][3] = {
        [0] = {
            [0] = 1,
            [1] = (pSamplingCoe[0][0] << 4) + pSamplingCoe[0][1],
            [2] = pEncode->nQTId[0],
        },
        [1] = {
            [0] = 2,
            [1] = (pSamplingCoe[1][0] << 4) + pSamplingCoe[1][1],
            [2] = pEncode->nQTId[1],
        },
        [2] = {
            [0] = 3,
            [1] = (pSamplingCoe[2][0] << 4) + pSamplingCoe[2][1],
            [2] = pEncode->nQTId[2],
        },
    };
    uint8_t* pData = pEncode->pData;
    HANSIZE nOffset = 0;

    switch (pEncode->jpegType) {
        case PICTURE_JPEG_TYPE_BASELINE: { pMarker[1] = 0xC0; } break;
        case PICTURE_JPEG_TYPE_PROGRESSIVE: { pMarker[1] = 0xC2; } break;
        default: { } break;
    }

    memcpy(&pData[nOffset], pMarker, 2); nOffset += 2;
    WriteJpegData2ByteMSB(&pData[nOffset], nLen); nOffset += 2;
    memcpy(&pData[nOffset], &nSamplePrecision, 1); nOffset += 1;
    memcpy(&pData[nOffset], pH, 2); nOffset += 2;
    memcpy(&pData[nOffset], pW, 2); nOffset += 2;
    memcpy(&pData[nOffset], &nComponent, 1); nOffset += 1;
    memcpy(&pData[nOffset], pComponentInfo, 9); nOffset += 9;

    WriteFile(hFile, pData, (DWORD)nOffset, NULL, NULL);
}
static void SaveJpegWriteDHT(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    uint8_t pMarker[2] = { 0xFF, 0xC4 };
    uint16_t nLen;
    HANSIZE nCnt;
    uint8_t* pData = pEncode->pData;
    HANSIZE nOffset = 0;

    memcpy(&pData[nOffset], pMarker, 2); nOffset += 4;

    switch (pEncode->jpegType) {
        case PICTURE_JPEG_TYPE_BASELINE: { pEncode->pHT = &g_pSaveJpegHuffmanTableInfo[PICTURE_SAVE_JPEG_HUFFMAN_TABLE_BASELINE_DEFAULT]; } break;
        case PICTURE_JPEG_TYPE_PROGRESSIVE: { pEncode->pHT = &g_pSaveJpegHuffmanTableInfo[PICTURE_SAVE_JPEG_HUFFMAN_TABLE_PROGRESSIVE_DEFAULT]; } break;
        default: { } break;
    }

    for (PICTUREJPEGSEGMENTDHTTABLETYPE eType = 0; eType < PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_CNT; eType++)
    {
        for (HANSIZE iLoop = 0; iLoop < pEncode->pHT->nCnt[eType]; iLoop++)
        {
            pData[nOffset] = (uint8_t)(((HANSIZE)eType << 4) + iLoop); nOffset += 1;
            nCnt = 0;
            for (HANSIZE jLoop = 0; jLoop < 16; jLoop++)
            {
                pData[nOffset] = pEncode->pHT->pHuffman[eType][iLoop]->pCnt[jLoop];
                nCnt += pData[nOffset];
                nOffset += 1;
            }
            memcpy(&pData[nOffset], pEncode->pHT->pHuffman[eType][iLoop]->pValue, nCnt); nOffset += nCnt;
        }
    }

    nLen = (uint16_t)(nOffset - 2);
    WriteJpegData2ByteMSB(&pData[2], nLen);
    
    WriteFile(hFile, pData, (DWORD)nOffset, NULL, NULL);
}
static void SaveJpegWriteSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    SaveJpegRGBToYCbCr(pEncode);
    SaveJpegSampleYCbCr(pEncode);
    EncodeSOS(pEncode, hFile);
}

static void SaveJpegRGBToYCbCr(PPICTUREJPEGENCODEMACHINE pEncode)
{
    HANPPICTURE pPicture = pEncode->pPicture;
    PPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    PPICTURERESOLUTION pSample = &(pEncode->pxSample);
    PPICTURERGBA* pRGBA = pPicture->pPictureMap;
    PPICTUREJPEGYCBCR* pYCbCrMap = pEncode->pYCbCrMap;
    uint32_t nX;
    uint32_t nY;

    for (uint32_t nRow = 0; nRow < pSample->pxHeight; nRow++)
    {
        if (pResolution->pxHeight <= nRow) { nY = pResolution->pxHeight - 1; }
        else { nY = nRow; }
        for (uint32_t nCol = 0; nCol < pSample->pxWidth; nCol++)
        {
            if (pResolution->pxWidth <= nCol) { nX = pResolution->pxWidth - 1; }
            else { nX = nCol; }
            pYCbCrMap[nRow][nCol][0] = (PICTUREJPEGNUMBER)(0.299 * pRGBA[nY][nX].r + 0.587 * pRGBA[nY][nX].g + 0.114 * pRGBA[nY][nX].b);
            pYCbCrMap[nRow][nCol][1] = (PICTUREJPEGNUMBER)(-0.16874 * pRGBA[nY][nX].r - 0.33126 * pRGBA[nY][nX].g + 0.5 * pRGBA[nY][nX].b + 128);
            pYCbCrMap[nRow][nCol][2] = (PICTUREJPEGNUMBER)(0.5 * pRGBA[nY][nX].r - 0.41869 * pRGBA[nY][nX].g - 0.08131 * pRGBA[nY][nX].b + 128);
        }
    }
}
static void SaveJpegSampleYCbCr(PPICTUREJPEGENCODEMACHINE pEncode)
{
    PPICTURERESOLUTION pMCUCnt = &(pEncode->pxMCUCnt);
    PPICTURERESOLUTION pMCUBlockCnt = &(pEncode->pxMCUBlockCnt);
    PPICTURERESOLUTION pMCUSize = &(pEncode->pxMCUSize);
    PICTURERESOLUTION pxPixelLeftTop;
    PICTURERESOLUTION pxPixel;
    uint8_t nComponent = pEncode->nComponent;
    const uint8_t* pSamplingCoe;
    uint8_t* pMCUComponentBlockCnt = pEncode->pMCUComponentBlockCnt;
    PPICTUREJPEGYCBCR* pYCbCrMap = pEncode->pYCbCrMap;
    PICTUREJPEGDCTMATRIX2D** pComponentSample = pEncode->pComponentSample;
    HANSIZE nMCUId;
    HANSIZE nBlockId;
    uint8_t pSampleStep[PICTURE_JPEG_SEGMENT_COMPONENT_MAX][2];

    for (uint8_t iLoop = 0; iLoop < nComponent; iLoop++)
    {
        pSampleStep[iLoop][0] = pMCUBlockCnt->pxWidth / pEncode->pSamplingCoe[iLoop][0];
        pSampleStep[iLoop][1] = pMCUBlockCnt->pxHeight / pEncode->pSamplingCoe[iLoop][1];
    }

    /* 遍历 MCU */
    for (uint32_t nMCURow = 0; nMCURow < pMCUCnt->pxHeight; nMCURow++)
    {
        for (uint32_t nMCUCol = 0; nMCUCol < pMCUCnt->pxWidth; nMCUCol++)
        {
            nMCUId = nMCURow * pMCUCnt->pxWidth + nMCUCol;
            /* 遍历 MCU 内的每个分量 */
            for (uint8_t nComponentId = 0; nComponentId < nComponent; nComponentId++)
            {
                pSamplingCoe = pEncode->pSamplingCoe[nComponentId];
                nBlockId = nMCUId * pMCUComponentBlockCnt[nComponentId];
                /* 遍历该分量的每个 Block */
                for (uint8_t nBlockRow = 0; nBlockRow < pSamplingCoe[1]; nBlockRow++)
                {
                    for (uint8_t nBlockCol = 0; nBlockCol < pSamplingCoe[0]; nBlockCol++)
                    {
                        /* 计算当前 Block 的左上角像素是图片的哪个位置 */
                        pxPixelLeftTop.pxWidth = (nMCUCol * pMCUSize->pxWidth) + (nBlockCol * pSampleStep[nComponentId][0] * 8);
                        pxPixelLeftTop.pxHeight = (nMCURow * pMCUSize->pxHeight) + (nBlockRow * pSampleStep[nComponentId][1] * 8);
                        pxPixel.pxHeight = pxPixelLeftTop.pxHeight;
                        for (uint8_t nRow = 0; nRow < 8; nRow++)
                        {
                            pxPixel.pxWidth = pxPixelLeftTop.pxWidth;
                            for (uint8_t nCol = 0; nCol < 8; nCol++)
                            {
                                pComponentSample[nComponentId][nBlockId][nRow][nCol] = pYCbCrMap[pxPixel.pxHeight][pxPixel.pxWidth][nComponentId];
                                pxPixel.pxWidth += pSampleStep[nComponentId][0];
                            }
                            pxPixel.pxHeight += pSampleStep[nComponentId][1];
                        }
                        nBlockId++;
                    }
                }
            }
        }
    }
}

static inline HANINT GetSegmentListWindowWidth(void)
{
    HANINT nRet = 0;
    for (PICTUREJPEGSEGMENTLISTHEADER iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_LIST_HEADER_CNT; iLoop++)
    {
        nRet += sg_pJpegSegmentListHeader[iLoop].nWidth;
    }
    return nRet;
}
static inline HANINT GetSegmentInfoWindowWidth(void)
{
    HANINT nRet = 0;
    for (PICTUREJPEGSEGMENTINFOHEADER iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_INFO_HEADER_CNT; iLoop++)
    {
        nRet += sg_pJpegSegmentInfoHeaderWidth[iLoop];
    }
    return nRet;
}

static HANSIZE ReadJpegSegment_Default(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment)
{
    HANSIZE nRet = 0;
    HANSIZE nLength;
    uint16_t cMarker;

    if (4 <= nLen)
    {
        cMarker = ReadJpegData2ByteMSB(&pData[0]);
        if (0xFF00 < cMarker)
        {
            nLength = (HANSIZE)ReadJpegData2ByteMSB(&pData[2]) + (HANSIZE)2;
            if ((4 <= nLength) && (nRet <= nLength))
            {
                pSegment->cMarker = ReadJpegData2ByteMSB(&pData[0]);
                pSegment->nLength = (uint16_t)(nLength - 4);
                pSegment->nExLength = 0;
                pSegment->pData = &pData[4];
                pSegment->pSegment = &pData[0];
                nRet = nLength;
            }
        }
    }

    return nRet;
}
static HANSIZE ReadJpegSegment_SOI(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment)
{
    HANSIZE nRet = 0;
    uint16_t cMarker;
    
    if (2 <= nLen)
    {
        cMarker = ReadJpegData2ByteMSB(&pData[0]);
        if (0xFFD8 == cMarker)
        {
            pSegment->cMarker = cMarker;
            pSegment->nLength = 0;
            pSegment->nExLength = 0;
            pSegment->pData = NULL;
            pSegment->pSegment = &pData[0];
            nRet = 2;
        }
    }

    return nRet;
}
static HANSIZE ReadJpegSegment_SOS(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment)
{
    HANSIZE nRet = 0;
    HANSIZE nLength;
    uint16_t cMarker;
    
    if (4 <= nLen)
    {
        cMarker = ReadJpegData2ByteMSB(&pData[0]);
        if (0xFFDA == cMarker)
        {
            nLength = (HANSIZE)ReadJpegData2ByteMSB(&pData[2]) + (HANSIZE)2;
            if (nLength <= nLen)
            {
                pSegment->cMarker = cMarker;
                pSegment->nLength = (uint16_t)(nLength - 4);
                pSegment->nExLength = GetNextSegmentPos(&pData[nLength], nLen - nLength);
                pSegment->pData = &pData[4];
                pSegment->pSegment = &pData[0];
                nRet = nLength + pSegment->nExLength;
            }
        }
    }

    return nRet;
}
static HANSIZE ReadJpegSegment_RSTn(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment)
{
    HANSIZE nRet = 0;
    uint16_t cMarker;
    
    if (2 <= nLen)
    {
        cMarker = ReadJpegData2ByteMSB(&pData[0]);
        if ((0xFFD0 <= cMarker) && (cMarker <= 0xFFD7))
        {
            pSegment->cMarker = cMarker;
            pSegment->nLength = 0;
            pSegment->nExLength = GetNextSegmentPos(&pData[2], nLen - 2);
            pSegment->pData = &pData[2];
            pSegment->pSegment = &pData[0];
            nRet = 2 + pSegment->nExLength;
        }
    }

    return nRet;
}
static HANSIZE ReadJpegSegment_EOI(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENT pSegment)
{
    HANSIZE nRet = 0;
    uint16_t cMarker;
    
    if (2 <= nLen)
    {
        cMarker = ReadJpegData2ByteMSB(&pData[0]);
        if (0xFFD9 == cMarker)
        {
            pSegment->cMarker = cMarker;
            pSegment->nLength = 0;
            pSegment->nExLength = 0;
            pSegment->pData = NULL;
            pSegment->pSegment = &pData[0];
            nRet = 2;
        }
    }

    return nRet;
}

static BOOL UpdateSegmentInfo_DQT(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = pSegment->pData;
    HANSIZE nTablePos;
    HANSIZE nOffset;
    uint8_t cValueType;
    uint8_t nValueSize;
    uint8_t nTableId;
    uint16_t nValue;

    pSegmentInfo->DQT.bValid = TRUE;
    nTablePos = 0;
    while (nTablePos < pSegment->nLength)
    {
        nOffset = nTablePos;
        cValueType = (pData[nOffset] >> 4) & 0x0F;
        nTableId = pData[nOffset] & 0x0F;
        
        if (PICTURE_JPEG_SEGMENT_QT_MAX <= nTableId) { bRet = FALSE; }

        if (0 == cValueType) { nValueSize = 1; }
        else if (1 == cValueType) { nValueSize = 2; }
        else { bRet = FALSE; }

        if (TRUE == bRet)
        {
            for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
            {
                for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
                {
                    nOffset = nTablePos + 1 + sg_pZigZagOrderTable[iLoop][jLoop] * nValueSize;
                    switch (nValueSize) {
                    case 1: { nValue = pData[nOffset]; } break;
                    case 2: { nValue = ReadJpegData2ByteMSB(&pData[nOffset]); } break;
                    default: { } break;
                    }
                    pSegmentInfo->DQT.pQT[nTableId][iLoop][jLoop] = nValue;
                }
            }
            if ((pSegmentInfo->DQT.nQTCnt) <= nTableId) { pSegmentInfo->DQT.nQTCnt = nTableId + 1; }
        }
        else { break; }

        nTablePos += 1 + 64 * nValueSize;
    }

    return bRet;
}
static BOOL UpdateSegmentInfo_DHT(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = pSegment->pData;
    HANSIZE nTablePos;
    HANSIZE nOffset;
    uint8_t cTableType;
    uint8_t nTableId;
    HANSIZE nCodeOffset;
    PICTUREJPEGHUFFMAN hHuffman;
    PPICTUREJPEGSEGMENTDHTINFO pDHT;
    uint8_t nDHTOffset;

    pSegmentInfo->DHT.bValid = TRUE;
    nTablePos = 0;
    while (nTablePos < pSegment->nLength)
    {
        nOffset = nTablePos;
        cTableType = (pData[nOffset] >> 4) & 0x0F;
        nTableId = pData[nOffset] & 0x0F;
        if ((cTableType < 2) && (nTableId < PICTURE_JPEG_SEGMENT_HT_MAX))
        {
            pDHT = &(pSegmentInfo->DHT.pHT[cTableType][nTableId]);
            /* 读取码数表 */
            memcpy(pDHT->pCnt, &pData[nOffset + 1], sizeof(pDHT->pCnt));
            /* 编码从 0 开始 */
            hHuffman.cCode = 0;
            /* 码表从 17 开始（表信息 1 Bit，码数表 16 Bit） */
            nCodeOffset = 17;
            /* DHT 码表从下标 0 开始 */
            nDHTOffset = 0;
            for (HANSIZE iLoop = 0; iLoop < sizeof(pDHT->pCnt); iLoop++)
            {
                if (0 == pDHT->pCnt[iLoop]) { pDHT->pOffset[iLoop] = 0xFF; }
                else { pDHT->pOffset[iLoop] = nDHTOffset; }
                for (uint8_t jLoop = 0; jLoop < pDHT->pCnt[iLoop]; jLoop++)
                {
                    hHuffman.nValue = pData[nOffset + nCodeOffset];
                    pDHT->pValue[nDHTOffset] = hHuffman.nValue;
                    pDHT->pCode[nDHTOffset] = hHuffman.cCode;
                    hHuffman.cCode++;
                    nCodeOffset++;
                    nDHTOffset++;
                }
                hHuffman.cCode <<= 1;
            }

            nTablePos += nCodeOffset;
        }
        else { break; }
    }

    return bRet;
}
static BOOL UpdateSegmentInfo_SOFn(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = pSegment->pData;
    HANSIZE nOffset;
    uint8_t nComponentId;
    uint8_t nCoeH;
    uint8_t nCoeV;
    uint8_t nMaxH = 0;
    uint8_t nMaxV = 0;
    uint8_t nBlockCnt;
    uint8_t nMCUComponentBlockOffset;

    if (0xFFC0 == pSegment->cMarker) { pSegmentInfo->eType = PICTURE_JPEG_TYPE_BASELINE; }
    else if (0xFFC2 == pSegment->cMarker) { pSegmentInfo->eType = PICTURE_JPEG_TYPE_PROGRESSIVE; }
    else { pSegmentInfo->eType = PICTURE_JPEG_TYPE_UNKNOWN; }
    pSegmentInfo->SOFn.bValid = TRUE;
    pSegmentInfo->SOFn.nBitDepth = pData[0];
    pSegmentInfo->SOFn.pxResolution.pxHeight = ReadJpegData2ByteMSB(&pData[1]);
    pSegmentInfo->SOFn.pxResolution.pxWidth = ReadJpegData2ByteMSB(&pData[3]);
    pSegmentInfo->SOFn.nComponentCnt = pData[5];
    /* 标准解法 */
    pSegmentInfo->SOFn.bStdComponent = TRUE;
    for (uint8_t iLoop = 0; iLoop < pSegmentInfo->SOFn.nComponentCnt; iLoop++)
    {
        nOffset = 6 + iLoop * 3;
        if (pData[nOffset] < ArrLen(sg_pComponentIdTable))
        {
            nComponentId = sg_pComponentIdTable[pData[nOffset]];
            if (nComponentId < PICTURE_JPEG_SEGMENT_COMPONENT_MAX)
            {
                nCoeH = (pData[nOffset + 1] >> 4) & 0x0F;
                nCoeV = pData[nOffset + 1] & 0x0F;
                pSegmentInfo->SOFn.pSamplingCoe[nComponentId][0] = nCoeH;
                pSegmentInfo->SOFn.pSamplingCoe[nComponentId][1] = nCoeV;
                pSegmentInfo->SOFn.pQuantTblId[nComponentId] = pData[nOffset + 2];
                if (nMaxH < nCoeH) { nMaxH = nCoeH; }
                if (nMaxV < nCoeV) { nMaxV = nCoeV; }
                pSegmentInfo->SOFn.pxMCUBlockCnt.pxWidth = nMaxH;
                pSegmentInfo->SOFn.pxMCUBlockCnt.pxHeight = nMaxV;
            }
            else
            {
                pSegmentInfo->SOFn.bStdComponent = FALSE;
                bRet = FALSE;
            }
        }
        else { bRet = FALSE; }

        if (FALSE == bRet) { break; }
    }
    /* 非标准分量 ID，尝试按照 0、1、2 的 ID 解码 */
    if (FALSE == bRet)
    {
        bRet = TRUE;
        for (uint8_t iLoop = 0; iLoop < pSegmentInfo->SOFn.nComponentCnt; iLoop++)
        {
            nOffset = 6 + iLoop * 3;
            nComponentId = pData[nOffset];
            /* 非标情况只能按照 0、1、2 的 ID 解码 */
            if (nComponentId < pSegmentInfo->SOFn.nComponentCnt)
            {
                nCoeH = (pData[nOffset + 1] >> 4) & 0x0F;
                nCoeV = pData[nOffset + 1] & 0x0F;
                pSegmentInfo->SOFn.pSamplingCoe[nComponentId][0] = nCoeH;
                pSegmentInfo->SOFn.pSamplingCoe[nComponentId][1] = nCoeV;
                pSegmentInfo->SOFn.pQuantTblId[nComponentId] = pData[nOffset + 2];
                if (nMaxH < nCoeH) { nMaxH = nCoeH; }
                if (nMaxV < nCoeV) { nMaxV = nCoeV; }
                pSegmentInfo->SOFn.pxMCUBlockCnt.pxWidth = nMaxH;
                pSegmentInfo->SOFn.pxMCUBlockCnt.pxHeight = nMaxV;
            }
            else
            {
                bRet = FALSE;
                break;
            }
        }
    }
    /* 计算 MCU 和块的信息 */
    if (TRUE == bRet)
    {
        pSegmentInfo->SOFn.pxMCUSize.pxWidth = nMaxH * 8;
        pSegmentInfo->SOFn.pxMCUSize.pxHeight = nMaxV * 8;
        pSegmentInfo->SOFn.nMCUComponentBlockTotal = 0;
        nBlockCnt = 0;
        nMCUComponentBlockOffset = 0;
        for (uint8_t iLoop = 0; iLoop < pSegmentInfo->SOFn.nComponentCnt; iLoop++)
        {
            pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop] = pSegmentInfo->SOFn.pSamplingCoe[iLoop][0] * pSegmentInfo->SOFn.pSamplingCoe[iLoop][1];
            pSegmentInfo->SOFn.nMCUComponentBlockTotal += pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop];
            for (uint8_t jLoop = 0; jLoop < pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop]; jLoop++)
            {
                pSegmentInfo->SOFn.pBlockComponentId[nBlockCnt] = iLoop;
                nBlockCnt++;
            }
            pSegmentInfo->SOFn.pMCUComponentBlockOffset[iLoop] = nMCUComponentBlockOffset;
            nMCUComponentBlockOffset += pSegmentInfo->SOFn.pMCUComponentBlockCnt[iLoop];
        }
        pSegmentInfo->SOFn.pxMCUCnt.pxWidth = pSegmentInfo->SOFn.pxResolution.pxWidth / pSegmentInfo->SOFn.pxMCUSize.pxWidth;
        pSegmentInfo->SOFn.pxMCUCnt.pxHeight = pSegmentInfo->SOFn.pxResolution.pxHeight / pSegmentInfo->SOFn.pxMCUSize.pxHeight;
        if (0 != (pSegmentInfo->SOFn.pxResolution.pxWidth % pSegmentInfo->SOFn.pxMCUSize.pxWidth)) { pSegmentInfo->SOFn.pxMCUCnt.pxWidth++; }
        if (0 != (pSegmentInfo->SOFn.pxResolution.pxHeight % pSegmentInfo->SOFn.pxMCUSize.pxHeight)) { pSegmentInfo->SOFn.pxMCUCnt.pxHeight++; }
        pSegmentInfo->SOFn.nBlockTotal = pSegmentInfo->SOFn.pxMCUCnt.pxWidth * pSegmentInfo->SOFn.pxMCUCnt.pxHeight * pSegmentInfo->SOFn.nMCUComponentBlockTotal;
    }

    return bRet;
}
static BOOL UpdateSegmentInfo_SOS(PPICTUREJPEGSEGMENT pSegment, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = pSegment->pData;
    uint8_t nComponentCnt = pData[0];
    uint8_t nComponentId;
    uint8_t nComponentDcId;
    uint8_t nComponentAcId;

    pSegmentInfo->SOS.bValid = TRUE;
    pSegmentInfo->SOS.nLen = pSegment->nLength;
    pSegmentInfo->SOS.pData = pSegment->pData;
    pSegmentInfo->SOS.nExLen = pSegment->nExLength;
    pSegmentInfo->SOS.pExData = pSegment->pExData;
    pSegmentInfo->SOS.nComponentCnt = pData[0];
    for (uint8_t iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_COMPONENT_MAX; iLoop++)
    {
        pSegmentInfo->SOS.pComponentValid[iLoop] = FALSE;
    }
    /* 标准解法 */
    for (uint8_t iLoop = 0; iLoop < nComponentCnt; iLoop++)
    {
        nComponentId = sg_pComponentIdTable[pData[1 + (iLoop * 2)]];
        if (nComponentId < pSegmentInfo->SOFn.nComponentCnt)
        {
            pSegmentInfo->SOS.pComponentValid[nComponentId] = TRUE;
            nComponentDcId = (pData[2 + (iLoop * 2)] >> 4) & 0x0F;
            nComponentAcId = pData[2 + (iLoop * 2)] & 0x0F;
            pSegmentInfo->SOS.pComponentTableId[nComponentId][PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC] = nComponentDcId;
            pSegmentInfo->SOS.pComponentTableId[nComponentId][PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC] = nComponentAcId;
            pSegmentInfo->SOS.pSOSTotal[nComponentId]++;
        }
        else
        {
            bRet = FALSE;
            break;
        }
    }
    /* 非标准分量 ID，尝试按照 0、1、2 的 ID 解码 */
    if (FALSE == bRet)
    {
        bRet = TRUE;
        for (uint8_t iLoop = 0; iLoop < nComponentCnt; iLoop++)
        {
            nComponentId = pData[1 + (iLoop * 2)];
            if (nComponentId < nComponentCnt)
            {
                pSegmentInfo->SOS.pComponentValid[nComponentId] = TRUE;
                nComponentDcId = (pData[2 + (iLoop * 2)] >> 4) & 0x0F;
                nComponentAcId = pData[2 + (iLoop * 2)] & 0x0F;
                pSegmentInfo->SOS.pComponentTableId[nComponentId][PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC] = nComponentDcId;
                pSegmentInfo->SOS.pComponentTableId[nComponentId][PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC] = nComponentAcId;
            pSegmentInfo->SOS.pSOSTotal[nComponentId]++;
            }
            else
            {
                bRet = FALSE;
                break;
            }
        }
    }
    pSegmentInfo->SOS.nSpectralStart = pData[1 + (nComponentCnt * 2)];
    pSegmentInfo->SOS.nSpectralEnd = pData[2 + (nComponentCnt * 2)];
    pSegmentInfo->SOS.nAh = (pData[3 + (nComponentCnt * 2)] >> 4) & 0x0F;
    pSegmentInfo->SOS.nAl = pData[3 + (nComponentCnt * 2)] & 0x0F;

    return bRet;
}

static void SetSegmentMap_Default(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment)
{
    if (NULL != *pLastSOS)
    {
        (*pLastSOS)->nExLength = pSegment->nPos - (*pLastSOS)->nPos - 4 - (*pLastSOS)->nLength;
    }
    pSegment->nLength = ReadJpegData2ByteMSB(&pData[2]) - 2;;
    pSegment->nExLength = 0;
    pSegment->pData = &pData[4];
    pSegment->pExData = NULL;
    *pLastSOS = NULL;
}
static BOOL SetSegmentMap_SOI_EOI(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment)
{
    (void)pData;
    BOOL bRet = FALSE;

    if ((0xFFD8 == pSegment->cMarker) || (0xFFD9 == pSegment->cMarker))
    {
        if (NULL != *pLastSOS)
        {
            (*pLastSOS)->nExLength = pSegment->nPos - (*pLastSOS)->nPos - 4 - (*pLastSOS)->nLength;
        }
        pSegment->nLength = 0;
        pSegment->nExLength = 0;
        pSegment->pData = NULL;
        pSegment->pExData = NULL;
        *pLastSOS = NULL;
        bRet = TRUE;
    }
    
    return bRet;
}
static BOOL SetSegmentMap_SOS(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment)
{
    BOOL bRet = FALSE;

    if (0xFFDA == pSegment->cMarker)
    {
        if (NULL != *pLastSOS)
        {
            (*pLastSOS)->nExLength = pSegment->nPos - (*pLastSOS)->nPos - 4 - (*pLastSOS)->nLength;
        }
        pSegment->nLength = ReadJpegData2ByteMSB(&pData[2]) - 2;
        pSegment->nExLength = 0;
        pSegment->pData = &pData[4];
        pSegment->pExData = &pData[4 + pSegment->nLength];
        *pLastSOS = pSegment;
        bRet = TRUE;
    }
    
    return bRet;
}
static BOOL SetSegmentMap_RSTn(const uint8_t* pData, PPICTUREJPEGSEGMENT* pLastSOS, PPICTUREJPEGSEGMENT pSegment)
{
    (void)pData;
    (void)pLastSOS;
    BOOL bRet = FALSE;

    if ((0xFFD0 <= pSegment->cMarker) && (pSegment->cMarker <= 0xFFD7))
    {
        pSegment->nLength = 0;
        pSegment->nExLength = 0;
        pSegment->pData = NULL;
        pSegment->pExData = NULL;
        bRet = TRUE;
    }
    
    return bRet;
}

static void UpdateSegmentInfoWindow_InsertBlankLine(HWND hListView)
{
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(hListView),
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
        .pszText = TEXT(""),
    };
    ListView_InsertItem(hListView, &lvItem);
}
static BOOL UpdateSegmentInfoWindow_Default(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = pText;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
    ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetJpeg_Default_Name();
    ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
    for (PICTUREJPEGSEGMENTFIELD_DEFAULT iLoop = 0; iLoop < PICTURE_JPEG_DEFAULT_SEGMENT_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetJpeg_Default_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }
    
    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 长度 */
    lvItem.iItem = PICTURE_JPEG_DEFAULT_SEGMENT_FIELD_LEN;
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pSegment->nLength);
    ListView_SetItem(hListView, &lvItem);
    /* 数据 */
    lvItem.iItem = PICTURE_JPEG_DEFAULT_SEGMENT_FIELD_DATA;
    PictureJpegPrintHexData(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, &(pSegment->pData[0]), pSegment->nLength);
    ListView_SetItem(hListView, &lvItem);
    /* 附加长度 */
    lvItem.iItem = PICTURE_JPEG_DEFAULT_SEGMENT_FIELD_EXLEN;
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSegment->nExLength);
    ListView_SetItem(hListView, &lvItem);
    /* 附加数据 */
    lvItem.iItem = PICTURE_JPEG_DEFAULT_SEGMENT_FIELD_EXDATA;
    PictureJpegPrintHexData(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, &(pSegment->pData[pSegment->nLength]), pSegment->nExLength);
    ListView_SetItem(hListView, &lvItem);

    return TRUE;
}
static BOOL UpdateSegmentInfoWindow_SOI(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    if (0xFFD8 == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_SOI_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_APPn(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    LVCOLUMN lvField =
    {
        .mask = LVCF_TEXT,
        .pszText = pText,
    };

    if ((0xFFE0 <= pSegment->cMarker) && (pSegment->cMarker <= 0xFFEF))
    {
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvField);

        if (TRUE == UpdateSegmentInfoWindow_APP0_ReadJFIF(pSegment, hListView)) { bRet = TRUE; }
        else if (TRUE == UpdateSegmentInfoWindow_APP1_ReadExif(pSegment, hListView)) { bRet = TRUE; }
        else if (TRUE == UpdateSegmentInfoWindow_APP1_ReadAdobeXMP(pSegment, hListView)) { bRet = TRUE; }
        else if (TRUE == UpdateSegmentInfoWindow_APP2_ReadMPF(pSegment, hListView)) { bRet = TRUE; }
        else if (TRUE == UpdateSegmentInfoWindow_APP2_ReadICCProfile(pSegment, hListView)) { bRet = TRUE; }
        else if (TRUE == UpdateSegmentInfoWindow_APP13_ReadPhotoshop(pSegment, hListView)) { bRet = TRUE; }
        else if (TRUE == UpdateSegmentInfoWindow_APP14_ReadAdobe(pSegment, hListView)) { bRet = TRUE; }
        else { bRet = FALSE; }
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_COM(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    const uint8_t* pData = pSegment->pData;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    HANSIZE nCommentLen = pSegment->nLength + 1;

    if (0xFFFE == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_COM_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
        for (PICTUREJPEGSEGMENTFIELD_COM iLoop = 0; iLoop < PICTURE_JPEG_COM_SEGMENT_FIELD_CNT; iLoop++)
        {
            lvItem.iItem = iLoop;
            lvItem.pszText = GetJpeg_COM_FieldName(iLoop);
            ListView_InsertItem(hListView, &lvItem);
        }
    
        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
        lvItem.pszText = pText;
        /* 注释 */
        lvItem.iItem = PICTURE_JPEG_COM_SEGMENT_FIELD_COMMENT;
        if (HAN_PICTURE_JPEG_TEXT_BUF_SIZE < nCommentLen) { nCommentLen = HAN_PICTURE_JPEG_TEXT_BUF_SIZE; }
        HAN_snprintf(pText, nCommentLen, HANPSTR_PRINT_PCHAR_FORMAT, pData);
        pText[nCommentLen - 1] = TEXT('\0');
        ListView_SetItem(hListView, &lvItem);

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_DQT(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    const uint8_t* pData = pSegment->pData;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    HANSIZE nTableCnt;
    HANSIZE nTablePos;
    HANSIZE nOffset;
    uint8_t cValueType;
    uint8_t nTableId;
    uint8_t nValueSize;
    uint16_t nValue;

    if (0xFFDB == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_DQT_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvItem.pszText = pText;
        nTableCnt = 0;
        nTablePos = 0;
        while (nTablePos < pSegment->nLength)
        {
            nOffset = nTablePos;
            cValueType = (pData[nOffset] >> 4) & 0x0F;
            nTableId = pData[nOffset] & 0x0F;
            
            if (0 == cValueType) { nValueSize = 1; }
            else if (1 == cValueType) { nValueSize = 2; }
            else { break; }

            if (0 < nTableCnt) { UpdateSegmentInfoWindow_InsertBlankLine(hListView); }
            lvItem.iItem = ListView_GetItemCount(hListView);

            for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
            {
                lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
                if (0 == iLoop) { HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("表%u"), nTableId); }
                else { HAN_strcpy(pText, TEXT("")); }
                ListView_InsertItem(hListView, &lvItem);

                lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
                HAN_strcpy(pText, TEXT(""));
                for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
                {
                    nOffset = nTablePos + 1 + sg_pZigZagOrderTable[iLoop][jLoop] * nValueSize;
                    switch (nValueSize) {
                        case 1: { nValue = pData[nOffset]; } break;
                        case 2: { nValue = ReadJpegData2ByteMSB(&pData[nOffset]); } break;
                        default: { } break;
                    }
                    HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%4u"), nValue);
                    pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
                    HAN_strcat(pText, pValue);
                    ListView_SetItem(hListView, &lvItem);
                }
                lvItem.iItem++;
            }

            nTableCnt++;
            nTablePos += 1 + 64 * nValueSize;
        }

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_DHT(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    const uint8_t* pData = pSegment->pData;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    uint8_t pCodeCnt[16];
    HANSIZE nTableCnt;
    HANSIZE nOffset;
    HANSIZE nTablePos;
    uint8_t cTableType;
    uint8_t nTableId;
    HANSIZE nCodeOffset;
    PICTUREJPEGHUFFMAN hHuffman;

    if (0xFFC4 == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_DHT_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvItem.pszText = pText;
        nTableCnt = 0;
        nTablePos = 0;
        while (nTablePos < pSegment->nLength)
        {
            nOffset = nTablePos;
            cTableType = (pData[nOffset] >> 4) & 0x0F;
            nTableId = pData[nOffset] & 0x0F;
            if ((cTableType < 2) && (nTableId < PICTURE_JPEG_SEGMENT_HT_MAX))
            {
                /* 打印表头 */
                if (0 < nTableCnt) { UpdateSegmentInfoWindow_InsertBlankLine(hListView); }
                lvItem.iItem = ListView_GetItemCount(hListView);
                lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
                HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s%u"), sg_pDHTTableType[cTableType], nTableId);
                ListView_InsertItem(hListView, &lvItem);
                lvItem.iItem++;
                /* 读取码数表 */
                memcpy(pCodeCnt, &pData[nOffset + 1], sizeof(pCodeCnt));
                /* 编码从 0 开始 */
                hHuffman.cCode = 0;
                /* 码表从 17 开始（表信息 1 Bit，码数表 16 Bit） */
                nCodeOffset = 17;
                for (HANSIZE iLoop = 0; iLoop < sizeof(pCodeCnt); iLoop++)
                {
                    /* 当前码长 */
                    hHuffman.nLen = (uint8_t)(iLoop + 1);
                    for (uint8_t jLoop = 0; jLoop < pCodeCnt[iLoop]; jLoop++)
                    {
                        hHuffman.nValue = pData[nOffset + nCodeOffset];

                        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
                        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%02X"), hHuffman.nValue);
                        ListView_InsertItem(hListView, &lvItem);

                        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
                        PrintHuffmanCode(&hHuffman, pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
                        ListView_SetItem(hListView, &lvItem);

                        hHuffman.cCode++;
                        nCodeOffset++;
                        lvItem.iItem++;
                    }
                    hHuffman.cCode <<= 1;
                }

                nTableCnt++;
                nTablePos += nCodeOffset;
            }
            else { break; }
        }

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_SOFn(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    const uint8_t* pData = pSegment->pData;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    uint16_t nHeight;
    uint16_t nWidth;
    uint8_t nComponentNum;
    HANSIZE nOffset;

    if ((0xFFC0 == pSegment->cMarker) || (0xFFC2 == pSegment->cMarker))
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_SOFn_Name(pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
        for (PICTUREJPEGSEGMENTFIELD_SOFn iLoop = 0; iLoop < PICTURE_JPEG_SOFn_SEGMENT_FIELD_CNT; iLoop++)
        {
            lvItem.iItem = iLoop;
            lvItem.pszText = GetJpeg_SOFn_FieldName(iLoop);
            ListView_InsertItem(hListView, &lvItem);
        }
    
        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
        lvItem.pszText = pText;
        /* 位深 */
        lvItem.iItem = PICTURE_JPEG_SOFn_SEGMENT_FIELD_BIT_DEPTH;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pData[0]);
        ListView_SetItem(hListView, &lvItem);
        /* 图像高度 */
        lvItem.iItem = PICTURE_JPEG_SOFn_SEGMENT_FIELD_HEIGHT;
        nHeight = ReadJpegData2ByteMSB(&pData[1]);
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nHeight);
        ListView_SetItem(hListView, &lvItem);
        /* 图像宽度 */
        lvItem.iItem = PICTURE_JPEG_SOFn_SEGMENT_FIELD_WIDTH;
        nWidth = ReadJpegData2ByteMSB(&pData[3]);
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nWidth);
        ListView_SetItem(hListView, &lvItem);
        /* 分量数 */
        nComponentNum = pData[5];
        lvItem.iItem = PICTURE_JPEG_SOFn_SEGMENT_FIELD_COMPONENT_NUM;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nComponentNum);
        ListView_SetItem(hListView, &lvItem);

        for (uint8_t iLoop = 0; iLoop < nComponentNum; iLoop++)
        {
            nOffset = 6 + iLoop * 3;
            /* 插入一行空白，好看 */
            lvItem.iItem = PICTURE_JPEG_SOFn_SEGMENT_FIELD_CNT + (iLoop * 4);
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
            HAN_strcpy(pText, TEXT(""));
            ListView_InsertItem(hListView, &lvItem);
            /* 组件名 */
            lvItem.iItem++;
            GetJpeg_SOFn_ComponentName(pData[nOffset], pText);
            ListView_InsertItem(hListView, &lvItem);
            /* 采样因子 */
            lvItem.iItem++;
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
            HAN_strcpy(pText, GetJpeg_SOFn_ComponentSamplingFactorName());
            ListView_InsertItem(hListView, &lvItem);
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
            HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u, %u"), (pData[nOffset + 1] >> 4) & 0x0F, pData[nOffset + 1] & 0x0F);
            ListView_SetItem(hListView, &lvItem);
            /* 量化表ID */
            lvItem.iItem++;
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
            HAN_strcpy(pText, GetJpeg_SOFn_ComponentQuantitativeTableIdName());
            ListView_InsertItem(hListView, &lvItem);
            lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
            HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pData[nOffset + 2]);
            ListView_SetItem(hListView, &lvItem);
        }

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_DRI(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    const uint8_t* pData = pSegment->pData;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    uint16_t nValue;

    if (0xFFDD == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_DRI_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
        for (PICTUREJPEGSEGMENTFIELD_DRI iLoop = 0; iLoop < PICTURE_JPEG_DRI_SEGMENT_FIELD_CNT; iLoop++)
        {
            lvItem.iItem = iLoop;
            lvItem.pszText = GetJpeg_DRI_FieldName(iLoop);
            ListView_InsertItem(hListView, &lvItem);
        }
    
        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
        lvItem.pszText = pText;
        /* 注释 */
        lvItem.iItem = PICTURE_JPEG_DRI_SEGMENT_FIELD_RESTART_INTERVAL;
        nValue = ReadJpegData2ByteMSB(pData);
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nValue);
        ListView_SetItem(hListView, &lvItem);

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_SOS(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    const uint8_t* pData = pSegment->pData;
    uint8_t nComponentCnt = pData[0];
    uint8_t nComponentDcId;
    uint8_t nComponentAcId;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    if (0xFFDA == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_SOS_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
        for (PICTUREJPEGSEGMENTFIELD_SOS iLoop = 0; iLoop < PICTURE_JPEG_SOS_SEGMENT_FIELD_CNT; iLoop++)
        {
            lvItem.iItem = iLoop;
            lvItem.pszText = GetJpeg_SOS_FieldName(iLoop);
            ListView_InsertItem(hListView, &lvItem);
        }
    
        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
        lvItem.pszText = pText;
        /* 分量数 */
        lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_COMPONENT_NUM;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nComponentCnt);
        ListView_SetItem(hListView, &lvItem);
        /* 分量表 */
        for (uint8_t iLoop = 0; iLoop < nComponentCnt; iLoop++)
        {
            nComponentDcId = (pData[2 + (iLoop * 2)] >> 4) & 0x0F;
            nComponentAcId = pData[2 + (iLoop * 2)] & 0x0F;
            switch (pData[1 + (iLoop * 2)]) {
                case 0x01: {
                    lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_COMPONENT_Y;
                    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("DC表%u  AC表%u"), nComponentDcId, nComponentAcId);
                } break;
                case 0x02: {
                    lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_COMPONENT_Cb;
                    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("DC表%u  AC表%u"), nComponentDcId, nComponentAcId);
                } break;
                case 0x03: {
                    lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_COMPONENT_Cr;
                    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("DC表%u  AC表%u"), nComponentDcId, nComponentAcId);
                } break;
            }
            ListView_SetItem(hListView, &lvItem);
        }
        /* 频谱开始位置 */
        lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_SPECTRAL_START;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pData[1 + (nComponentCnt * 2)]);
        ListView_SetItem(hListView, &lvItem);
        /* 频谱结束位置 */
        lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_SPECTRAL_END;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pData[2 + (nComponentCnt * 2)]);
        ListView_SetItem(hListView, &lvItem);
        /* 渐近位 */
        lvItem.iItem = PICTURE_JPEG_SOS_SEGMENT_FIELD_PROGRESSIVE;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u,%u"),
            (pData[3 + (nComponentCnt * 2)] >> 4) & 0x0F, pData[3 + (nComponentCnt * 2)] & 0x0F
        );
        ListView_SetItem(hListView, &lvItem);

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_RSTn(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pNum[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    if ((0xFFD0 <= pSegment->cMarker) && (pSegment->cMarker <= 0xFFD7))
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        HAN_strcpy(pText, GetJpeg_RSTn_Name());
        HAN_snprintf(pNum, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pSegment->cMarker & 0x0F);
        pNum[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
        HAN_strcat(pText, pNum);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        bRet = TRUE;
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_EOI(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    if (0xFFD9 == pSegment->cMarker)
    {
        lvTitle.pszText = pText;
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), pSegment->cMarker);
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD, &lvTitle);
        lvTitle.pszText = GetJpeg_EOI_Name();
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        bRet = TRUE;
    }

    return bRet;
}

#if 1 /* 解码 */
static BOOL DecodeSOS(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    BOOL bRet;
    
    switch (jpegInfo->segment.segmentInfo.eType) {
        case PICTURE_JPEG_TYPE_BASELINE: { bRet = DecodeBaselineSOS(jpegInfo); } break;
        case PICTURE_JPEG_TYPE_PROGRESSIVE: { bRet = DecodeProgressiveSOS(jpegInfo); } break;
        default: { bRet = FALSE; } break;
    }
    sg_pDecodeProcedureMode[PICTURE_JPEG_DECODE_PROCEDURE_MODE_AUTO].DecodeCallback(jpegInfo);

    return bRet;
}
/* 参考了规范文档《CCITT Rec. T.81 (1992 E)》
 * 以下代码的注释中所参考的规范皆可查阅该文档
 */
#if 1 /* 基线式 JPEG */
static BOOL DecodeBaselineSOS(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    BOOL bRet;
    HANSIZE nSegmentCnt = jpegInfo->segment.map.nCnt;
    PPICTUREJPEGSEGMENT pSegment = jpegInfo->segment.map.pSegmentList;
    /* 基线型 JPEG 只有 1 个 SOS 可解，所以更新完所有图片信息后直接解码即可 */
    for (HANSIZE iLoop = 0; iLoop < nSegmentCnt; iLoop++) { UpdateSegmentInfo(&pSegment[iLoop], &(jpegInfo->segment.segmentInfo)); }

    bRet = DecodeBaselineHuffman(jpegInfo, &(jpegInfo->segment.segmentInfo));
    BaselineDequantizeBlock(jpegInfo);

    return bRet;
}
static BOOL DecodeBaselineHuffman(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    BOOL bLoop = TRUE;
    const uint8_t* pData = pSegmentInfo->SOS.pExData;
    HANSIZE nLen = pSegmentInfo->SOS.nExLen;
    const uint8_t* pHuffman;
    PICTUREJPEGREADBITRET eBitRet = PICTURE_JPEG_READ_BIT_RET_NROMAL;
    PICTUREJPEGDECODEMACHINE decodeMachine = {
        .rbBits = JpegInitReadBit(pData, nLen),
        .pSegmentInfo = pSegmentInfo,
        .nBlockId = 0,
        .nBlockSum = 0,
        .nCurrentComponent = 0,
        .pDecodedData1D = jpegInfo->pictureData.pDecodedData1D,
        .pDecodedData2D = jpegInfo->pictureData.pDecodedData2D,
        .pIDCT = jpegInfo->pictureData.pIDCTData,
        .nSpectral = 0,
        .pPredQT = { 0, },
    };

    for (uint8_t iLoop = 0; iLoop < jpegInfo->segment.segmentInfo.SOFn.nComponentCnt; iLoop++) { InitComponentMCUInfo(jpegInfo, &decodeMachine, iLoop); }
    decodeMachine.pHT = SwitchComponentAndHuffmanTable(&decodeMachine, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC);

    while (TRUE == bLoop)
    {
        bLoop = FALSE;
        /* 查找 Huffman 码 */
        pHuffman = NULL;
        while (JpegGetBitsLen(&(decodeMachine.rbBits)) < 16)
        {
            eBitRet = JpegReadBit(&(decodeMachine.rbBits));
            if ((PICTURE_JPEG_READ_BIT_RET_NROMAL == eBitRet) || (PICTURE_JPEG_READ_BIT_RET_RSTN == eBitRet))
            {
                pHuffman = DecodeHuffman((uint16_t)(decodeMachine.rbBits.cCode), decodeMachine.rbBits.nCodeLen, decodeMachine.pHT);
            }
            if ((PICTURE_JPEG_READ_BIT_RET_NROMAL != eBitRet) || (NULL != pHuffman)) { break; }
        }
        /* 解码 Huffman 码 */
        if (NULL != pHuffman)
        {
            JpegClearBit(&(decodeMachine.rbBits));
            if (0 == decodeMachine.nSpectral) { bLoop = DecodeBaselineDCHuffman(*pHuffman, &decodeMachine); }
            else { bLoop = DecodeBaselineACHuffman(*pHuffman, &decodeMachine); }
            JpegClearBit(&(decodeMachine.rbBits));
        }
        if (PICTURE_JPEG_READ_BIT_RET_RSTN == eBitRet)
        {
            if (0 == decodeMachine.nSpectral)
            {
                DecodeRSTnCallback(&decodeMachine);
                bLoop = TRUE;
            }
            else { bLoop = FALSE; }
        }
    }

    return bRet;
}
static inline BOOL DecodeBaselineDCHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode)
{
    BOOL bRet = FALSE;

    if (nValue <= 0x0F)
    {
        if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBits(&(pDecode->rbBits), nValue))
        {
            pDecode->pPredQT[pDecode->nCurrentComponent] += DecodeVLI(pDecode->rbBits.cCode, nValue);
            pDecode->pDecodedData1D[pDecode->nBlockId][0] = pDecode->pPredQT[pDecode->nCurrentComponent];
            pDecode->pHT = SwitchComponentAndHuffmanTable(pDecode, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC);
            pDecode->nSpectral = 1;
            bRet = TRUE;
        }
    }

    return bRet;
}
static inline BOOL DecodeBaselineACHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode)
{
    BOOL bRet = TRUE;
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    uint8_t nRRRR;
    uint8_t nSSSS;
    PICTUREJPEGREADBITRET eBitRet;

    if (0x00 == nValue) // EOB，直接进入下一个块
    {
        DecodeBaselineBlockOkCallback(pDecode);
    }
    else if (0xF0 == nValue) // ZRL，跳过 16 个系数
    {
        pDecode->nSpectral += 16;
        if (64 <= pDecode->nSpectral) { bRet = FALSE; }
    }
    else
    {
        nRRRR = (nValue >> 4) & 0x0F;
        nSSSS = nValue & 0x0F;
        if (0 < nSSSS) // 标准的 RRRRSSSS 格式中 SSSS 必须大于 0 （参考规范 F.1.2.2.1 的图 F.1，第 90 页）
        {
            pDecode->nSpectral += nRRRR;
            if (pDecode->nSpectral < 64)
            {
                eBitRet = JpegReadBits(&(pDecode->rbBits), nSSSS);
                if ((PICTURE_JPEG_READ_BIT_RET_NROMAL == eBitRet) || (PICTURE_JPEG_READ_BIT_RET_RSTN == eBitRet))
                {
                    (*pDecodedData1D)[pDecode->nSpectral] = DecodeVLI(pDecode->rbBits.cCode, nSSSS);
                    pDecode->nSpectral++;
                    if (64 == pDecode->nSpectral)
                    {
                        DecodeBaselineBlockOkCallback(pDecode);
                        if (PICTURE_JPEG_READ_BIT_RET_RSTN == eBitRet) { DecodeRSTnCallback(pDecode); }
                    }
                }
                else { bRet = FALSE; }
            }
            else {bRet = FALSE;}
        }
        else { bRet = FALSE; }
    }

    return bRet;
}
static inline void DecodeRSTnCallback(PPICTUREJPEGDECODEMACHINE pDecode)
{
    memset(pDecode->pPredQT, 0, sizeof(pDecode->pPredQT));
    JpegClearBit(&(pDecode->rbBits));
}
static inline void DecodeBaselineBlockOkCallback(PPICTUREJPEGDECODEMACHINE pDecode)
{
    BaselineZigZag(pDecode);
    RecordMCUInfoEnd(pDecode); // 切换分量前先记录旧分量的长度
    pDecode->nBlockId++;
    pDecode->nBlockSum++;
    pDecode->pHT = SwitchComponentAndHuffmanTable(pDecode, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC);
    RecordMCUInfoStart(pDecode); // 切换完了分量，记录新分量的起始位置
    pDecode->nSpectral = 0;
}
static inline void BaselineZigZag(PPICTUREJPEGDECODEMACHINE pDecode)
{
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    PICTUREJPEGDCTMATRIX2D* pDecodedData2D = &(pDecode->pDecodedData2D[pDecode->nBlockId]);
    HANSIZE nOffset;

    for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
    {
        for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
        {
            nOffset = sg_pZigZagOrderTable[iLoop][jLoop];
            (*pDecodedData2D)[iLoop][jLoop] = (*pDecodedData1D)[nOffset];
        }
    }
}
static void BaselineDequantizeBlock(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = &(jpegInfo->segment.segmentInfo);
    PICTUREJPEGQUANTTABLE* pQT = pSegmentInfo->DQT.pQT;
    PICTUREJPEGDCTMATRIX2D* pDecodedData2D = jpegInfo->pictureData.pDecodedData2D;
    uint8_t nComponentTotal = pSegmentInfo->SOFn.nMCUComponentBlockTotal;
    uint8_t* pBlockComponentId = pSegmentInfo->SOFn.pBlockComponentId;
    HANSIZE nBlockCnt = pSegmentInfo->SOFn.nBlockTotal;
    uint8_t nComponentId;
    uint8_t nQTId;

    for (HANSIZE nBlockId = 0; nBlockId < nBlockCnt; nBlockId++)
    {
        nComponentId = pBlockComponentId[nBlockId % nComponentTotal];
        nQTId = pSegmentInfo->SOFn.pQuantTblId[nComponentId];
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
            {
                pDecodedData2D[nBlockId][iLoop][jLoop] *= pQT[nQTId][iLoop][jLoop];
            }
        }
    }
}
#endif
#if 1 /* 渐进式 JPEG */
/* 渐进式编码与基线式编码的主要区别在于：
 * 1.   DCT 变换后的频谱分开进行编码，DC 与 AC 总是分开编码，不同频率的 AC 也可以分开编码（参考规范 G.1.1.1.1，第121页）
 * 2.   每个频谱允许多次扫描，第一次扫描可以对最高 N 比特进行编码，后续的细化扫描每次只编码 1 比特
 *      每次扫描时（包括首次扫描和后续细化扫描），先将 DCT 变换后每个频谱的值除以 2 的 Al 次方（规范中将这个右移操作称为点变换，参考124页第2段）
 *      然后取最低 N 位。对于首次扫描，N 就是商的全部位，对于细化扫描，N 就是最低位（也就是细化扫描只编码 1 比特）
 * 3.   对点变换后的结果进行编码
 */
static BOOL DecodeProgressiveSOS(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    BOOL bRet = TRUE;
    HANSIZE nSegmentCnt = jpegInfo->segment.map.nCnt;
    PPICTUREJPEGSEGMENT pSegment = jpegInfo->segment.map.pSegmentList;
    HANSIZE nSOSCnt = 0;
    uint8_t nSs;
    uint8_t nSe;
    uint8_t nAh;
    uint8_t nAl;
    /* 渐进式解码器主控程序基于以下规则：
     * 1.   DC 系数与 AC 系数总是分开编码（参考规范 G.1.1.1.1，第121页）
     *      所以 DC 解码和 AC 解码分别调用不同的函数
     * 2.   对于任意频带，都应从首次扫描（即 SOS 的 Ah = 0）开始，之后的扫描都应是逐 bit 细化（参考规范 G.1.1.1.2，第121页）
     *      所以任意频带的首次扫描和细化扫描分别调用不同的函数
     * 基于以上规则，每次解码 SOS 的压缩数据时，根据 Ss、Se、Ah、Al 的情况，分别调用 4 种函数，且只会调用其中一种
     * 分别是 DC 首次扫描、DC 细化扫描、AC 首次扫描、AC 细化扫描
     */
    for (HANSIZE iLoop = 0; iLoop < nSegmentCnt; iLoop++)
    {
        UpdateSegmentInfo(&pSegment[iLoop], &(jpegInfo->segment.segmentInfo));
        if (0xFFDA == pSegment[iLoop].cMarker)
        {
            nSs = jpegInfo->segment.segmentInfo.SOS.nSpectralStart;
            nSe = jpegInfo->segment.segmentInfo.SOS.nSpectralEnd;
            nAh = jpegInfo->segment.segmentInfo.SOS.nAh;
            nAl = jpegInfo->segment.segmentInfo.SOS.nAl;

            if ((0 != nAh) && (1 != (nAh - nAl))) // 对于任意频带，首次扫描只要求 nAh = 0，后续扫描都必须逐 bit 细化（nAh - nAl = 1）
            {
                bRet = FALSE;
            }
            else
            {
                if ((nSs == 0) && (nSe == 0)) // 扫描 DC
                {
                    if (0 == nAh) // 首次扫描 DC
                    {
                        /* 首次扫描 DC 的方式与基线式 JPEG 基本相同（参考规范 G.1.1.2.1，第121页）
                         * 主要区别在于：若 Al 不为 0
                         *      从编码器视角，需要将量化后的数据右移 Al 位，然后将移位后的 DC 值按基线式方法进行 Huffman 编码
                         *      从解码器的视角，则是将 Huffman 解码后的值移位后再进行反量化
                         *      若 Al 为 0，移 0 位和不移结果相同，所以这里不区分 Al
                         *      此外，由于移位和加减法的运算特性，DC 差值预测是放在移位前还是移位后，没有区别（先移位后累加和先累加后移位结果相同）
                         */
                        bRet = DecodeProgressiveDCFirstScan(jpegInfo, &(jpegInfo->segment.segmentInfo));
                    }
                    else // 细化扫描 DC
                    {
                        /* 无论是对 DC 第几次细化
                         * 总是对移位后的最低有效位进行编码（参考规范 G.1.1.2.1，第121页）
                         * 且不压缩（参考规范 G.1.2.1，第 2 段，第121页）
                         * 即 DC 细化扫描是一个纯比特流，按照分量 0、1、2、0、1、2 …… 的顺序排列，每个比特就是对应分量移位前的第 Al 位
                         * 所以使用一个函数实现
                         */
                        bRet = DecodeProgressiveDCSubsequentScan(jpegInfo, &(jpegInfo->segment.segmentInfo));
                    }
                }
                else if (0 < nSs) // 扫描 AC
                {
                    if (0 == nAh) // 首次扫描某段频谱的 AC
                    {
                        /* 首次扫描 AC 的方式与基线式 JPEG 相似（参考规范 G.1.2.2，第121页）
                         * 但在处理 Huffman 解码后的 RUN-SIZE 码时，有如下区别：
                         * 1.   若 SIZE ≠ 0，按正常的 RUN-SIZE 模式解码，即高 4 位是频谱值为 0 的个数，低 4 位是非 0 频谱需要读取的 bit 数
                         * 2.   若 SIZE = 0 且 SIZE ≠ 15（也就是非 ZRL），则按 EOBRUN 的模式解码
                         *      表示从当前 block 开始的 1 << RUN + Read(RUN) 个 block 都是 EOB，
                         *      Read(RUN) 就是读 RUN 个 bit，不做 VLI 转码
                         * 3.   ZRL（RUN = 15，SIZE = 0）正常解码，不按 EOBRUN 解码
                         * 4.   备注：在基线式编码中，除了 EOB 和 ZRL 外，不可能出现 SIZE = 0（参考规范 F.1.2.2.1，第90页）
                         *      因为 SIZE 为 0 的 VLI 码其值必然是 0，而 0 值应当在 RUN 中表述，不应在 SIZE 中体现，所以 0x10 ~ 0xE0（低 4 字节为 0）在基线式编码中是非法的
                         *      因此 0x10 ~ 0xE0 在渐进式编码中用来描述 EOBRUN，与渐进式不冲突
                         *      0x00 既可以表示 EOB，也可以表示 EOBRUN 中的 EOB0，两者效果相同
                         */
                        bRet = DecodeProgressiveACScan(jpegInfo, &(jpegInfo->segment.segmentInfo), DecodeProgressiveACFirstScanHuffman);
                    }
                    else // 细化扫描 AC
                    {
                        /* 细化扫描 AC 的 Huffman 码还是一样的查表法，但是 RUN-SIZE 的作用已经有很大的不同
                         * 先对规范中提到的名词 zero history 和 non-zero history 做一下解释：
                         *      zero history: 零历史，指某个系数在之前的扫描的结果一直是 0
                         *      non-zero history: 非零历史，指某个系数在之前的扫描已经得到了非 0 值
                         * 零历史具有如下特点：
                         * 1.   如果零历史在某次扫描中初次得到非 0 值时，由于细化扫描只编码只编码 1 比特，所以这个值一定是 ±1，（参考规范 G.1.2.3，规则a，第125页）
                         * 2.   Huffman 解码出来的 SIZE 指的就是这个 ±1 的 VLI 码的长度，它必然为 1（参考规范 G.1.2.3，第3段对 RRRRSSSS 的解释，第125页）
                         * 3.   如果零历史在这次扫描中结果还是 0，那么它会被统计进入 RUN（参考规范 G.1.2.3，第3段对 RRRRSSSS 的解释，第125页）
                         * 非零历史具有如下特点：
                         * 1.   一旦某个系数有值了，那么在后续扫描中它就永远都是非零历史了（非 0 的数即使加上校正比特也不可能再变回 0 ）
                         * 2.   由于非零历史已经有非 0 值了，所以校正位的解码方式就是简单的加上校正比特，不必再做 VLI 转码（参考规范 G.1.2.3，规则b，第125页）
                         * 3.   非零历史不再参与 RUN 的统计，也不由附加位来修正，它由附加位后面的校正位来修正
                         * 根据规范，所有编码的非 0 系数（即 RUN-SIZE 中的 SIZE 所对应的系数）都是指从 0 值变成非 0 值，并且由于只细化 1 比特，所以这个非 0 值一定是 ±1
                         * RUN-SIZE 中的 RUN，ZRL，EOB，EOBRUN 计数的 0 都是针对在当前扫描中仍然是 0 的系数。
                         * 而非零历史系数（也就是在先前的扫描中已经有非 0 值的系数）则是作为校正位添加在这些编码的后面
                         */
                        bRet = DecodeProgressiveACScan(jpegInfo, &(jpegInfo->segment.segmentInfo), DecodeProgressiveACSubsequentScanHuffman);
                    }
                }
                else
                {
                    bRet = FALSE;
                }
            }
            nSOSCnt++;
        }
    }

    ProgressiveDequantizeBlock(jpegInfo);

    return bRet;
}
static BOOL DecodeProgressiveDCFirstScan(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    BOOL bLoop = TRUE;
    const uint8_t* pData = pSegmentInfo->SOS.pExData;
    HANSIZE nLen = pSegmentInfo->SOS.nExLen;
    const uint8_t* pHuffman;
    uint8_t nId;
    PICTUREJPEGREADBITRET eBitRet;
    PICTUREJPEGDECODEMACHINE decodeMachine = {
        .rbBits = JpegInitReadBit(pData, nLen),
        .pSegmentInfo = pSegmentInfo,
        .nBlockTotal = pSegmentInfo->SOFn.nBlockTotal,
        .nBlockId = 0,
        .nCurrentComponent = 0,
        .pDecodedData1D = jpegInfo->pictureData.pDecodedData1D,
        .pDecodedData2D = jpegInfo->pictureData.pDecodedData2D,
        .pIDCT = jpegInfo->pictureData.pIDCTData,
        .nSs = pSegmentInfo->SOS.nSpectralStart,
        .nSe = pSegmentInfo->SOS.nSpectralEnd,
        .nAl = pSegmentInfo->SOS.nAl,
        .nSpectral = pSegmentInfo->SOS.nSpectralStart,
        .pPredQT = { 0, },
    };

    for (uint8_t iLoop = 0; iLoop < jpegInfo->segment.segmentInfo.SOFn.nComponentCnt; iLoop++) { InitComponentMCUInfo(jpegInfo, &decodeMachine, iLoop); }
    nId = decodeMachine.pSegmentInfo->SOS.pComponentTableId[decodeMachine.nCurrentComponent][PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC];
    decodeMachine.pHT = &(decodeMachine.pSegmentInfo->DHT.pHT[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][nId]);
    
    while (TRUE == bLoop)
    {
        bLoop = FALSE;
        /* 查找 Huffman 码 */
        pHuffman = NULL;
        while (JpegGetBitsLen(&(decodeMachine.rbBits)) < 16)
        {
            eBitRet = JpegReadBit(&(decodeMachine.rbBits));
            if (PICTURE_JPEG_READ_BIT_RET_NROMAL == eBitRet)
            {
                pHuffman = DecodeHuffman((uint16_t)(decodeMachine.rbBits.cCode), decodeMachine.rbBits.nCodeLen, decodeMachine.pHT);
            }
            if ((PICTURE_JPEG_READ_BIT_RET_NROMAL != eBitRet) || (NULL != pHuffman)) { break; }
        }
        /* 解码 Huffman 码 */
        if (NULL != pHuffman)
        {
            JpegClearBit(&(decodeMachine.rbBits));
            if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBits(&(decodeMachine.rbBits), *pHuffman))
            {
                DecodeProgressiveDCHuffmanValue((uint16_t)(decodeMachine.rbBits.cCode), decodeMachine.rbBits.nCodeLen, &decodeMachine);
                JpegClearBit(&(decodeMachine.rbBits));
                bLoop = TRUE;
            }
        }
    }

    return bRet;
}
static BOOL DecodeProgressiveDCSubsequentScan(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = pSegmentInfo->SOS.pExData;
    HANSIZE nLen = pSegmentInfo->SOS.nExLen;
    PICTUREJPEGDECODEMACHINE decodeMachine = {
        .rbBits = JpegInitReadBit(pData, nLen),
        .pSegmentInfo = pSegmentInfo,
        .nBlockTotal = pSegmentInfo->SOFn.nBlockTotal,
        .nBlockId = 0,
        .pDecodedData1D = jpegInfo->pictureData.pDecodedData1D,
        .pDecodedData2D = jpegInfo->pictureData.pDecodedData2D,
        .nAl = pSegmentInfo->SOS.nAl,
    };

    for (uint8_t iLoop = 0; iLoop < jpegInfo->segment.segmentInfo.SOFn.nComponentCnt; iLoop++) { InitComponentMCUInfo(jpegInfo, &decodeMachine, iLoop); }

    for (HANSIZE iLoop = 0; iLoop < decodeMachine.nBlockTotal; iLoop++)
    {
        if (PICTURE_JPEG_READ_BIT_RET_NROMAL != JpegReadBit(&(decodeMachine.rbBits))) { break; }
        if (0 != decodeMachine.rbBits.cCode)
        {
            decodeMachine.pDecodedData1D[decodeMachine.nBlockId][0] |= 1 << decodeMachine.nAl;
            decodeMachine.pDecodedData2D[decodeMachine.nBlockId][0][0] |= 1 << decodeMachine.nAl;
        }
        RecordMCUInfoEnd(&decodeMachine); // 切换分量前先记录旧分量的长度
        decodeMachine.nBlockId++;
        decodeMachine.nBlockSum++;
        SwitchComponent(&decodeMachine);
        RecordMCUInfoStart(&decodeMachine); // 切换完了分量，记录新分量的起始位置
        JpegClearBit(&(decodeMachine.rbBits));
    }

    return bRet;
}
static BOOL DecodeProgressiveACScan(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGSEGMENTINFO pSegmentInfo, DECODEPROGRESSIVEACHUFFMAN DecodeCallback)
{
    BOOL bRet = TRUE;
    BOOL bLoop = TRUE;
    const uint8_t* pData = pSegmentInfo->SOS.pExData;
    HANSIZE nLen = pSegmentInfo->SOS.nExLen;
    const uint8_t* pHuffman;
    uint8_t nId;
    BOOL* pComponentValid = pSegmentInfo->SOS.pComponentValid;
    PICTUREJPEGREADBITRET eBitRet;
    PICTUREJPEGDECODEMACHINE decodeMachine = {
        .rbBits = JpegInitReadBit(pData, nLen),
        .pSegmentInfo = pSegmentInfo,
        .nBlockTotal = pSegmentInfo->SOFn.nBlockTotal,
        .nMCUComponentBlockTotal = pSegmentInfo->SOFn.nMCUComponentBlockTotal,
        .pMCUComponentBlockCnt = pSegmentInfo->SOFn.pMCUComponentBlockCnt,
        .pMCUComponentBlockOffset = pSegmentInfo->SOFn.pMCUComponentBlockOffset,
        .pDecodedData1D = jpegInfo->pictureData.pDecodedData1D,
        .pDecodedData2D = jpegInfo->pictureData.pDecodedData2D,
        .pIDCT = jpegInfo->pictureData.pIDCTData,
        .nSs = pSegmentInfo->SOS.nSpectralStart,
        .nSe = pSegmentInfo->SOS.nSpectralEnd,
        .nAl = pSegmentInfo->SOS.nAl,
        .nSpectral = pSegmentInfo->SOS.nSpectralStart,
    };

    /* 确认正在扫描的分量，因为 AC 系数一次只能扫描一个分量，所以这里确认完后面就不会变了 */
    for (uint8_t iLoop = 0; iLoop < 3; iLoop++)
    {
        if (TRUE == pComponentValid[iLoop])
        {
            decodeMachine.nCurrentComponent = iLoop;
            decodeMachine.nBlockId = pSegmentInfo->SOFn.pMCUComponentBlockOffset[iLoop];
            break;
        }
    }
    /* 确认 Huffman 表，因为 AC 系数一次只能扫描一个分量，所以这里确认完后面就不会变了 */
    nId = decodeMachine.pSegmentInfo->SOS.pComponentTableId[decodeMachine.nCurrentComponent][PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC];
    decodeMachine.pHT = &(decodeMachine.pSegmentInfo->DHT.pHT[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][nId]);

    InitProgressiveDecodeMachineParam(jpegInfo, &decodeMachine);
    
    while (TRUE == bLoop)
    {
        bLoop = FALSE;
        /* 查找 Huffman 码 */
        pHuffman = NULL;
        while (JpegGetBitsLen(&(decodeMachine.rbBits)) < 16)
        {
            eBitRet = JpegReadBit(&(decodeMachine.rbBits));
            if (PICTURE_JPEG_READ_BIT_RET_NROMAL == eBitRet)
            {
                pHuffman = DecodeHuffman((uint16_t)(decodeMachine.rbBits.cCode), decodeMachine.rbBits.nCodeLen, decodeMachine.pHT);
            }
            if ((PICTURE_JPEG_READ_BIT_RET_NROMAL != eBitRet) || (NULL != pHuffman)) { break; }
        }
        /* 解码 Huffman 码 */
        if (NULL != pHuffman)
        {
            JpegClearBit(&(decodeMachine.rbBits));
            bLoop = DecodeCallback(*pHuffman, &decodeMachine);
            bRet = bLoop;
            JpegClearBit(&(decodeMachine.rbBits));
        }
    }

    return bRet;
}
static void InitProgressiveDecodeMachineParam(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGDECODEMACHINE pDecode)
{
    uint8_t nComponent = pDecode->nCurrentComponent;
    uint8_t* pSamplingCoe = pDecode->pSegmentInfo->SOFn.pSamplingCoe[nComponent];
    PPICTURERESOLUTION pResolution = &(jpegInfo->pictureData.pPictureInfo->pPicture[0]->pxResolution);
    PPICTURERESOLUTION pMCUSize = &(pDecode->pSegmentInfo->SOFn.pxMCUSize);
    PICTURERESOLUTION pxBlockSize;
    PICTURERESOLUTION pxBlockCnt;

    pxBlockSize.pxWidth = pMCUSize->pxWidth / pSamplingCoe[0];
    pxBlockSize.pxHeight = pMCUSize->pxHeight / pSamplingCoe[1];
    pxBlockCnt.pxWidth = (pResolution->pxWidth + pxBlockSize.pxWidth - 1) / pxBlockSize.pxWidth;
    pxBlockCnt.pxHeight = (pResolution->pxHeight + pxBlockSize.pxHeight - 1) / pxBlockSize.pxHeight;

    pDecode->progressiveParam.pMCUBlockCnt[nComponent].pxWidth = pSamplingCoe[0];
    pDecode->progressiveParam.pMCUBlockCnt[nComponent].pxHeight = pSamplingCoe[1];
    pDecode->progressiveParam.pPictureBlockCnt[nComponent] = pxBlockCnt;

    InitComponentMCUInfo(jpegInfo, pDecode, pDecode->nCurrentComponent);
}
static inline void DecodeProgressiveDCHuffmanValue(uint16_t nValue, uint8_t nLen, PPICTUREJPEGDECODEMACHINE pDecode)
{
    pDecode->pPredQT[pDecode->nCurrentComponent] += DecodeVLI(nValue, nLen);
    pDecode->pDecodedData1D[pDecode->nBlockId][0] = pDecode->pPredQT[pDecode->nCurrentComponent] << pDecode->nAl;
    pDecode->pDecodedData2D[pDecode->nBlockId][0][0] = pDecode->pDecodedData1D[pDecode->nBlockId][0];
    RecordMCUInfoEnd(pDecode); // 切换分量前先记录旧分量的长度
    pDecode->nBlockId++;
    pDecode->nBlockSum++;
    pDecode->pHT = SwitchComponentAndHuffmanTable(pDecode, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC);
    RecordMCUInfoStart(pDecode); // 切换完了分量，记录新分量的起始位置
}
static BOOL DecodeProgressiveACFirstScanHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode)
{
    BOOL bRet = TRUE;
    PPICTUREJPEGREADBIT pReadBit = &(pDecode->rbBits);
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    uint8_t nAl = pDecode->nAl;
    uint8_t nOffset = pDecode->nSpectral;
    uint8_t nRRRR;
    uint8_t nSSSS;
    HANSIZE nEOBRUN;
    
    nRRRR = (nValue >> 4) & 0x0F;
    nSSSS = nValue & 0x0F;
    if (0xF0 == nValue) { pDecode->nSpectral += 16; } // ZRL
    else if (0 == nSSSS) // EOBRUN
    {
        if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBits(pReadBit, nRRRR))
        {
            nEOBRUN = (1 << nRRRR) + pReadBit->cCode; // EOBRUN 读到的后续数据只有正数，不用再做 VLI 解码
            for (HANSIZE iLoop = 0; iLoop < nEOBRUN; iLoop++)
            {
                bRet = DecodeProgressiveACBlockOkCallback(pDecode);
                if (FALSE == bRet) { break; }
            }
        }
        else { bRet = FALSE; }
    }
    else // 标准 RRRRSSSS
    {
        if ((nOffset + nRRRR) <= pDecode->nSe)
        {
            pDecode->nSpectral += nRRRR;
            if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBits(pReadBit, nSSSS))
            {
                (*pDecodedData1D)[pDecode->nSpectral] = DecodeVLI(pReadBit->cCode, nSSSS) << nAl;
                if (pDecode->nSpectral < pDecode->nSe) { pDecode->nSpectral++; }
                else if (pDecode->nSpectral == pDecode->nSe) { bRet = DecodeProgressiveACBlockOkCallback(pDecode); }
                else { bRet = FALSE; }
            }
            else { bRet = FALSE; }
        }
        else { bRet = FALSE; }
    }

    return bRet;
}
static BOOL DecodeProgressiveACSubsequentScanHuffman(uint8_t nValue, PPICTUREJPEGDECODEMACHINE pDecode)
{
    BOOL bRet = TRUE;
    PPICTUREJPEGREADBIT pReadBit = &(pDecode->rbBits);
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    uint8_t nAl = pDecode->nAl;
    uint8_t nRRRR;
    uint8_t nSSSS;
    HANSIZE nEOBRUN;
    uint8_t cAdditionalBit;
    
    nRRRR = (nValue >> 4) & 0x0F;
    nSSSS = nValue & 0x0F;
    if (0xF0 == nValue) { bRet = ProgressiveACSubsequentScanJumpZRL(pDecode); } // ZRL
    else if (0 == nSSSS) // EOBRUN 或 EOB
    {
        if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBits(pReadBit, nRRRR))
        {
            nEOBRUN = (1 << nRRRR) + pReadBit->cCode;
            bRet = ProgressiveACSubsequentScanJumpEOBRUN(pDecode, nEOBRUN);
        }
        else { bRet = FALSE; }
    }
    else if (1 == nSSSS) // 标准 RRRRSSSS
    {
        if (PICTURE_JPEG_READ_BIT_RET_NROMAL != JpegReadBit(pReadBit)) { bRet = FALSE; } // 读一个附加位
        if (TRUE == bRet) // 跳过 RRRR 个 0 系数
        {
            cAdditionalBit = pReadBit->cCode;
            JpegClearBit(pReadBit);
            bRet = ProgressiveACSubsequentScanJumpRRRR(pDecode, nRRRR);
        }
        if (TRUE == bRet) // 赋值新的非 0 系数
        {
            (*pDecodedData1D)[pDecode->nSpectral] = DecodeVLI(cAdditionalBit, 1) << nAl;
            if (pDecode->nSpectral < pDecode->nSe) { pDecode->nSpectral++; }
            else if (pDecode->nSpectral == pDecode->nSe) { bRet = DecodeProgressiveACBlockOkCallback(pDecode); }
            else { bRet = FALSE; }
        }
        else { bRet = FALSE; }
    }
    else { bRet = FALSE; }

    return bRet;
}
static inline BOOL DecodeProgressiveACBlockOkCallback(PPICTUREJPEGDECODEMACHINE pDecode)
{
    BOOL bRet;

    if (pDecode->nBlockId < pDecode->nBlockTotal)
    {
        ProgressiveZigZag(pDecode);
        pDecode->nBlockSum++;
        RecordMCUInfoEnd(pDecode); // 切换分量前先记录旧分量的长度
        ProgressiveUpdateBlockId(pDecode);
        RecordMCUInfoStart(pDecode); // 切换完了分量，记录新分量的起始位置
        pDecode->nSpectral = pDecode->nSs;

        bRet = TRUE;
    }
    else { bRet = FALSE; }

    return bRet;
}
static inline void ProgressiveUpdateBlockId(PPICTUREJPEGDECODEMACHINE pDecode)
{
    HANSIZE nBlockSum = pDecode->nBlockSum;
    uint8_t nComponent = pDecode->nCurrentComponent;
    uint8_t nMCUComponentBlockOffset = pDecode->pSegmentInfo->SOFn.pMCUComponentBlockOffset[nComponent];
    PPICTURERESOLUTION pPictureBlockCnt = &(pDecode->progressiveParam.pPictureBlockCnt[nComponent]);
    PPICTURERESOLUTION pMCUBlockCnt = &(pDecode->progressiveParam.pMCUBlockCnt[nComponent]);
    PPICTURERESOLUTION pMCUCnt = &(pDecode->pSegmentInfo->SOFn.pxMCUCnt);
    HANSIZE nBlockRowId;
    HANSIZE nBlockColId;
    HANSIZE nMCURowId;
    HANSIZE nMCUColId;
    HANSIZE nMCUId;
    HANSIZE nMCUBlockOffset;

    nBlockRowId = nBlockSum / pPictureBlockCnt->pxWidth;    // 计算新的 Block 在第几行
    nBlockColId = nBlockSum % pPictureBlockCnt->pxWidth;    // 计算新的 Block 在第几列
    nMCURowId = nBlockRowId / pMCUBlockCnt->pxHeight;       // 计算新的 Block 行号对应第几行 MCU
    nMCUColId = nBlockColId / pMCUBlockCnt->pxWidth;        // 计算新的 Block 列号对应第几列 MCU
    nMCUId = nMCURowId * pMCUCnt->pxWidth + nMCUColId;      // 计算新的 Block 属于第几个 MCU
    nMCUBlockOffset = nBlockRowId % pMCUBlockCnt->pxHeight * pMCUBlockCnt->pxWidth + nBlockColId % pMCUBlockCnt->pxWidth + nMCUComponentBlockOffset; // 计算 Block 是所属 MCU 的第几个
    pDecode->nBlockId = nMCUId * pDecode->nMCUComponentBlockTotal + nMCUBlockOffset;

    pDecode->pMCUInfo[nComponent]->nBlockCnt = nMCUId * pDecode->pMCUComponentBlockCnt[nComponent] + nMCUBlockOffset;
}
static inline void ProgressiveZigZag(PPICTUREJPEGDECODEMACHINE pDecode)
{
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    PICTUREJPEGDCTMATRIX2D* pDecodedData2D = &(pDecode->pDecodedData2D[pDecode->nBlockId]);
    HANSIZE nOffset;
    uint8_t nSs = pDecode->nSs;
    uint8_t nSe = pDecode->nSe;

    for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
    {
        for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
        {
            nOffset = sg_pZigZagOrderTable[iLoop][jLoop];
            if ((nSs <= nOffset) && (nOffset <= nSe))
            {
                (*pDecodedData2D)[iLoop][jLoop] = (*pDecodedData1D)[nOffset];
            }
        }
    }
}
static void ProgressiveDequantizeBlock(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = &(jpegInfo->segment.segmentInfo);
    PICTUREJPEGQUANTTABLE* pQT = pSegmentInfo->DQT.pQT;
    PICTUREJPEGDCTMATRIX2D* pDecodedData2D = jpegInfo->pictureData.pDecodedData2D;
    uint8_t nComponentTotal = pSegmentInfo->SOFn.nMCUComponentBlockTotal;
    uint8_t* pBlockComponentId = pSegmentInfo->SOFn.pBlockComponentId;
    HANSIZE nBlockCnt = pSegmentInfo->SOFn.nBlockTotal;
    uint8_t nComponentId;
    uint8_t nQTId;
    
    for (HANSIZE nBlockId = 0; nBlockId < nBlockCnt; nBlockId++)
    {
        nComponentId = pBlockComponentId[nBlockId % nComponentTotal];
        nQTId = pSegmentInfo->SOFn.pQuantTblId[nComponentId];
        for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
        {
            for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
            {
                pDecodedData2D[nBlockId][iLoop][jLoop] *= pQT[nQTId][iLoop][jLoop];
            }
        }
    }
}
static inline BOOL ProgressiveACSubsequentScanJumpRRRR(PPICTUREJPEGDECODEMACHINE pDecode, uint8_t nRRRR)
{
    BOOL bRet = TRUE;
    PPICTUREJPEGREADBIT pReadBit = &(pDecode->rbBits);
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    uint8_t nAl = pDecode->nAl;
    uint8_t nZeroCnt = 0;
    uint8_t nOffset = pDecode->nSpectral;

    while ((nZeroCnt < nRRRR) && (nOffset < pDecode->nSe)) // 跳过 Run 个 0 系数，因为要留一个频率给附加位，所以频率不可能到上限
    {
        if (0 == (*pDecodedData1D)[nOffset]) { nZeroCnt++; } // 零历史，要跳过 RRRR 个
        else // 非零历史，因为 ZRL 不会跟附加位，所以直接读一个校正位即可
        {
            if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBit(pReadBit))
            {
                (*pDecodedData1D)[nOffset] |= pReadBit->cCode << nAl;
                JpegClearBit(pReadBit);
            }
            else  { break; }
        }
        nOffset++;
    }
    if ((nZeroCnt < nRRRR) || (pDecode->nSe < nOffset)) { bRet = FALSE; } // 任何错误中断都会导致 0 计数不够 16
    else // 跳过剩下的非 0 系数
    {
        while (TRUE == bRet)
        {
            if (0 == (*pDecodedData1D)[nOffset]) // 找到了附加位对应的频率，结束循环
            {
                pDecode->nSpectral = nOffset;
                break;
            }
            else
            {
                if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBit(pReadBit))
                {
                    (*pDecodedData1D)[nOffset] |= pReadBit->cCode << nAl;
                    JpegClearBit(pReadBit);
                }
                else  { bRet = FALSE; }
            }
            if (pDecode->nSe <= nOffset) { bRet = FALSE; } // 直到频率结束也没找到附加位对应的频率
            nOffset++;
        }
    }

    return bRet;
}
static inline BOOL ProgressiveACSubsequentScanJumpZRL(PPICTUREJPEGDECODEMACHINE pDecode)
{
    BOOL bRet = TRUE;
    PPICTUREJPEGREADBIT pReadBit = &(pDecode->rbBits);
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    uint8_t nAl = pDecode->nAl;
    uint8_t nZeroCnt = 0;
    uint8_t nOffset = pDecode->nSpectral;

    while ((nZeroCnt < 16) && (nOffset <= pDecode->nSe)) // 跳过 0 值并修正非零历史系数
    {
        if (0 == (*pDecodedData1D)[nOffset]) { nZeroCnt++; } // 零历史，要跳过 RRRR 个
        else // 非零历史，因为 ZRL 不会跟附加位，所以直接读一个校正位即可
        {
            if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBit(pReadBit))
            {
                (*pDecodedData1D)[nOffset] |= pReadBit->cCode << nAl;
                JpegClearBit(pReadBit);
            }
            else  { break; }
        }
        nOffset++;
    }
    if ((nZeroCnt < 16) || (pDecode->nSe < nOffset)) { bRet = FALSE; } // 任何错误中断都会导致 0 计数不够 16
    else { pDecode->nSpectral = nOffset; }

    return bRet;
}
static inline BOOL ProgressiveACSubsequentScanJumpEOBRUN(PPICTUREJPEGDECODEMACHINE pDecode, HANSIZE nEOBRUN)
{
    BOOL bRet = TRUE;
    PPICTUREJPEGREADBIT pReadBit = &(pDecode->rbBits);
    PICTUREJPEGDCTMATRIX1D* pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
    uint8_t nAl = pDecode->nAl;
    uint8_t nOffset = pDecode->nSpectral;

    for (HANSIZE iLoop = 0; iLoop < nEOBRUN; iLoop++)
    {
        pDecodedData1D = &(pDecode->pDecodedData1D[pDecode->nBlockId]);
        for (nOffset = pDecode->nSpectral; nOffset < pDecode->nSe; nOffset++)
        {
            if (0 != (*pDecodedData1D)[nOffset])
            {
                if (PICTURE_JPEG_READ_BIT_RET_NROMAL == JpegReadBit(pReadBit))
                {
                    (*pDecodedData1D)[nOffset] |= pReadBit->cCode << nAl;
                    JpegClearBit(pReadBit);
                }
                else 
                {
                    bRet = FALSE;
                    break;
                }
            }
        }
        if (TRUE == bRet) { bRet = DecodeProgressiveACBlockOkCallback(pDecode); }
        if (FALSE == bRet) { break; }
    }

    return bRet;
}
#endif
/* 读 bit、解 bit */
static PICTUREJPEGREADBIT JpegInitReadBit(const uint8_t* pData, HANSIZE nLen)
{
    PICTUREJPEGREADBIT rbRet = {
        .pData = pData,
        .nDataLen = nLen,
        .cData = pData[0],
        .iByte = 0,
        .iBit = 0,
        .cCode = 0,
        .nCodeLen = 0,
    };

    if ((0xFF == pData[0]) && (0x00 == pData[1])) { rbRet.iByte++; }

    return rbRet;
}
static inline void JpegClearBit(PPICTUREJPEGREADBIT pReadBit)
{
    pReadBit->cCode = 0;
    pReadBit->nCodeLen = 0;
}
static inline uint8_t JpegGetBitsLen(PPICTUREJPEGREADBIT pReadBit)
{
    return pReadBit->nCodeLen;
}
static inline PICTUREJPEGREADBITRET JpegReadBit(PPICTUREJPEGREADBIT pReadBit)
{
    PICTUREJPEGREADBITRET eRet = PICTURE_JPEG_READ_BIT_RET_NROMAL;
    uint8_t cBit = (pReadBit->cData << pReadBit->iBit) & 0x80;
    PICTUREJPEGREADBIT rbTmp;

    if (pReadBit->iByte < pReadBit->nDataLen)
    {
        pReadBit->cCode <<= 1;
        if (0 != cBit) { pReadBit->cCode |= (uint32_t)1; }
        pReadBit->nCodeLen++;
        pReadBit->iBit++;

        if (8 <= pReadBit->iBit)
        {
            pReadBit->iBit = 0;
            pReadBit->iByte++;
            pReadBit->cData = pReadBit->pData[pReadBit->iByte];
            if (0xFF == pReadBit->cData)
            {
                if (pReadBit->iByte < pReadBit->nDataLen) // 还有可以解码的字节
                {
                    pReadBit->iByte++;
                    if ((pReadBit->nDataLen - 1) <= pReadBit->iByte) // 字节不够，无法解码
                    {
                        eRet = PICTURE_JPEG_READ_BIT_RET_ERROR;
                    }
                    else if (0x00 == pReadBit->pData[pReadBit->iByte]) // 0xFF00
                    {
                        eRet = PICTURE_JPEG_READ_BIT_RET_NROMAL;
                    }
                    else if ((0xD0 <= pReadBit->pData[pReadBit->iByte]) && (pReadBit->pData[pReadBit->iByte] <= 0xD7)) // RSTn，需要调用者重置后续数据
                    {
                        pReadBit->iByte++;
                        rbTmp = JpegInitReadBit(&(pReadBit->pData[pReadBit->iByte]), pReadBit->nDataLen - pReadBit->iByte);
                        rbTmp.cCode = pReadBit->cCode;
                        rbTmp.nCodeLen = pReadBit->nCodeLen;
                        *pReadBit = rbTmp;
                        eRet = PICTURE_JPEG_READ_BIT_RET_RSTN;
                    }
                    else // 非法的 0xFF
                    {
                        eRet = PICTURE_JPEG_READ_BIT_RET_ERROR;
                    }
                }
                else // 当前数据已经结束，0xFF 是下一个数据段的数据了，结束解码
                {
                    eRet = PICTURE_JPEG_READ_BIT_RET_NROMAL;
                }
            }
        }
    }
    else // 正常结束
    {
        eRet = PICTURE_JPEG_READ_BIT_RET_END;
    }

    return eRet;
}
static inline PICTUREJPEGREADBITRET JpegReadBits(PPICTUREJPEGREADBIT pReadBit, uint8_t nLen)
{
    PICTUREJPEGREADBITRET eRet = PICTURE_JPEG_READ_BIT_RET_NROMAL;

    for (uint8_t iLoop = 0; iLoop < nLen; iLoop++)
    {
        eRet = JpegReadBit(pReadBit);
        if ((eRet != PICTURE_JPEG_READ_BIT_RET_NROMAL) && (iLoop < (nLen - 1)))
        {
            eRet = PICTURE_JPEG_READ_BIT_RET_ERROR;
            break;
        }
    }

    return eRet;
}
static inline HANSIZE GetBitCnt(HANSIZE nByteStart, uint8_t nBitStart, HANSIZE nByteEnd, uint8_t nBitEnd)
{
    return ((nByteEnd - nByteStart) << 3) + nBitEnd - nBitStart;
}
static inline uint8_t* DecodeHuffman(uint16_t cCode, uint8_t nCodeLen, PPICTUREJPEGSEGMENTDHTINFO pHuffmanTable)
{
    uint8_t * pRet = NULL;
    uint8_t nId = nCodeLen - 1;
    uint8_t nOffset = pHuffmanTable->pOffset[nId];

    if (0xFF != nOffset)
    {
        for (HANSIZE iLoop = 0; iLoop < pHuffmanTable->pCnt[nId]; iLoop++)
        {
            if (pHuffmanTable->pCode[nOffset + iLoop] == cCode)
            {
                pRet = &(pHuffmanTable->pValue[nOffset + iLoop]);
                break;
            }
        }
    }

    return pRet;
}
static inline int16_t DecodeVLI(uint16_t cCode, uint8_t nLen)
{
    int16_t nRet;
    int32_t cMask;
    int32_t nValue = (int32_t)cCode;
    
    if (0 < nLen)
    {
        cMask = 1 << (nLen - 1);
        if (0 == (nValue & cMask)) { nValue = nValue - (1 << nLen) + 1; }
        nRet = (int16_t)nValue;
    }
    else { nRet = 0; }

    return nRet;
}
/* 不同解码模式共用的代码 */
static void InitComponentMCUInfo(PPICTUREJPEGWNDEXTRA jpegInfo, PPICTUREJPEGDECODEMACHINE pDecode, uint8_t nComponent)
{
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = pDecode->pSegmentInfo;
    PPICTUREJPEGMCUINFO pMCUInfo = jpegInfo->pictureData.mcuTable[nComponent].pInfo;

    for (HANSIZE iLoop = 0; iLoop < jpegInfo->pictureData.mcuTable[nComponent].nCnt; iLoop++)
    {
        if (FALSE == pMCUInfo[iLoop].bValid)
        {
            pDecode->pMCUInfo[nComponent] = &pMCUInfo[iLoop];
            pMCUInfo[iLoop].nComponent = nComponent;
            pMCUInfo[iLoop].nSs = pSegmentInfo->SOS.nSpectralStart;
            pMCUInfo[iLoop].nSe = pSegmentInfo->SOS.nSpectralEnd;
            pMCUInfo[iLoop].nAh = pSegmentInfo->SOS.nAh;
            pMCUInfo[iLoop].nAl = pSegmentInfo->SOS.nAl;
            pMCUInfo[iLoop].pBlockPos[0].nByteStart = 0;
            pMCUInfo[iLoop].pBlockPos[0].nBitStart = 0;
            pMCUInfo[iLoop].pBlockPos[0].nBitCnt = 0;
            pMCUInfo[iLoop].nBlockCnt = 0;
            pMCUInfo[iLoop].bValid = TRUE;
            break;
        }
    }
}
static void RecordMCUInfoStart(PPICTUREJPEGDECODEMACHINE pDecode)
{
    PPICTUREJPEGMCUINFO pMCUInfo = pDecode->pMCUInfo[pDecode->nCurrentComponent];
    PPICTUREJPEGBLOCKPOS pPos = &(pMCUInfo->pBlockPos[pMCUInfo->nBlockCnt]);

    pPos->nByteStart = pDecode->rbBits.iByte;
    pPos->nBitStart = pDecode->rbBits.iBit;
}
static void RecordMCUInfoEnd(PPICTUREJPEGDECODEMACHINE pDecode)
{
    PPICTUREJPEGMCUINFO pMCUInfo = pDecode->pMCUInfo[pDecode->nCurrentComponent];
    PPICTUREJPEGBLOCKPOS pPos = &(pMCUInfo->pBlockPos[pMCUInfo->nBlockCnt]);
    
    pPos->nBitCnt = GetBitCnt(pPos->nByteStart, pPos->nBitStart, pDecode->rbBits.iByte, pDecode->rbBits.iBit);
    pPos->bValid = TRUE;
    pMCUInfo->nBlockCnt++;
}
static inline void SwitchComponent(PPICTUREJPEGDECODEMACHINE pDecode)
{
    uint8_t nBlockId = pDecode->nBlockId % pDecode->pSegmentInfo->SOFn.nMCUComponentBlockTotal;
    
    pDecode->nCurrentComponent = pDecode->pSegmentInfo->SOFn.pBlockComponentId[nBlockId];
}
static inline PPICTUREJPEGSEGMENTDHTINFO SwitchComponentAndHuffmanTable(PPICTUREJPEGDECODEMACHINE pDecode, PICTUREJPEGSEGMENTDHTTABLETYPE eDcAc)
{
    PPICTUREJPEGSEGMENTDHTINFO pRet;
    uint8_t nId;
    
    SwitchComponent(pDecode);
    nId = pDecode->pSegmentInfo->SOS.pComponentTableId[pDecode->nCurrentComponent][eDcAc];
    pRet = &(pDecode->pSegmentInfo->DHT.pHT[eDcAc][nId]);

    return pRet;
}
static void MapJpegPixels(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PICTUREJPEGDCTMATRIX2D* pIDCT = jpegInfo->pictureData.pIDCTData;
    PPICTUREJPEGYCBCR pYCbCr = jpegInfo->pictureData.pYCbCrData;
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = &(jpegInfo->segment.segmentInfo);
    PPICTURERESOLUTION pMCUSize = &(pSegmentInfo->SOFn.pxMCUSize);
    PPICTURERESOLUTION pMCUCnt = &(pSegmentInfo->SOFn.pxMCUCnt);
    uint8_t* pMCUComponentBlockCnt = pSegmentInfo->SOFn.pMCUComponentBlockCnt;
    uint8_t nComponentCnt = pSegmentInfo->SOFn.nComponentCnt;

    /* 遍历 MCU 的循环按像素数算 */
    HANSIZE nMCUColStep = pMCUSize->pxWidth;
    HANSIZE nMCUColMax = nMCUColStep * pMCUCnt->pxWidth;
    HANSIZE nMCURowStep = nMCUColMax * pMCUSize->pxHeight;
    HANSIZE nMCURowMax = nMCURowStep * pMCUCnt->pxHeight;

    HANSIZE nIDCTId = 0;
    HANSIZE nYCbCrId;

    PICTURERESOLUTION pxBlockSize[3];
    PICTURERESOLUTION pDivId[3];
    HANSIZE nIDCTX;
    HANSIZE nIDCTY;
    HANSIZE nComponenBlockX[4] = { 0, 8, 0, 8, };
    HANSIZE nComponentBlockY[4] = { 0, 0, 8 * nMCUColMax, 8 * nMCUColMax, };

    for (HANSIZE iLoop = 0; iLoop < nComponentCnt; iLoop++)
    {
        pxBlockSize[iLoop].pxWidth = pMCUSize->pxWidth / pSegmentInfo->SOFn.pSamplingCoe[iLoop][0];
        pxBlockSize[iLoop].pxHeight = pMCUSize->pxHeight / pSegmentInfo->SOFn.pSamplingCoe[iLoop][1];
        pDivId[iLoop].pxWidth = (pxBlockSize[iLoop].pxWidth / 8) - 1;
        pDivId[iLoop].pxHeight = (pxBlockSize[iLoop].pxHeight / 8) - 1;
    }

    /* 遍历 MCU，这里的 pYCbCr 已经补上缺少的像素，因此不用考虑超范围 */
    for (HANSIZE nMCURow = 0; nMCURow < nMCURowMax; nMCURow += nMCURowStep)
    {
        for (HANSIZE nMCUCol = 0; nMCUCol < nMCUColMax; nMCUCol += nMCUColStep)
        {
            /* 遍历分量 */
            for (HANSIZE nComponentId = 0; nComponentId < nComponentCnt; nComponentId++)
            {
                /* 遍历分量的 block */
                for (HANSIZE nBlockId = 0; nBlockId < pMCUComponentBlockCnt[nComponentId]; nBlockId++)
                {
                    /* 固定 block 左上角的 YCbCr ID */
                    nYCbCrId = (nMCURow + nComponentBlockY[nBlockId]) + (nMCUCol + nComponenBlockX[nBlockId]);
                    /* 遍历 block，对于 1:1 采样的 block，遍历次数是 8 × 8
                     * 对于非 1:1 采样的 block，遍历次数可能是 8 × 16 或 16 × 16，此时 IDCT 的行列 ID 需要查表获取
                     * 为了统一，无论是几比几，都要查表
                     */
                    for (HANSIZE nBlockY = 0; nBlockY < pxBlockSize[nComponentId].pxHeight; nBlockY++)
                    {
                        nIDCTY = sg_pRestorIdTable[pDivId[nComponentId].pxHeight][nBlockY];
                        for (HANSIZE nBlockX = 0; nBlockX < pxBlockSize[nComponentId].pxWidth; nBlockX++)
                        {
                            nIDCTX = sg_pRestorIdTable[pDivId[nComponentId].pxWidth][nBlockX];
                            pYCbCr[nYCbCrId + nBlockX][nComponentId] = pIDCT[nIDCTId][nIDCTY][nIDCTX];
                        }
                        /* YCbCr 切换到下一行 */
                        nYCbCrId += nMCUColMax;
                    }
                    nIDCTId++;
                }
            }
        }
    }
}
static void ConvertYCbCrToRGB(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANPPICTURE pPicture = jpegInfo->pictureData.pPictureInfo->pPicture[0];
    PPICTUREJPEGYCBCR pYCbCr = jpegInfo->pictureData.pYCbCrData;
    uint32_t pxWidth = pPicture->pxResolution.pxWidth;
    uint32_t pxHeight = pPicture->pxResolution.pxHeight;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    int32_t y;
    int32_t u;
    int32_t v;
    HANSIZE nYCbCrId = 0;
    uint32_t nSkip = jpegInfo->segment.segmentInfo.SOFn.pxMCUCnt.pxWidth * jpegInfo->segment.segmentInfo.SOFn.pxMCUSize.pxWidth - pxWidth;

    if (1 == jpegInfo->segment.segmentInfo.SOFn.nComponentCnt)
    {
        for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
        {
            for (uint32_t ncol = 0; ncol < pxWidth; ncol++)
            {
                pPictureMap[nRow][ncol].r = (uint8_t)pYCbCr[nYCbCrId][0];
                pPictureMap[nRow][ncol].g = (uint8_t)pYCbCr[nYCbCrId][0];
                pPictureMap[nRow][ncol].b = (uint8_t)pYCbCr[nYCbCrId][0];
                pPictureMap[nRow][ncol].a = 0xFF;
                nYCbCrId++;
            }
            /* 跳过边界 MCU 像素 */
            nYCbCrId += nSkip;
        }
    }
    else
    {
        for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
        {
            for (uint32_t ncol = 0; ncol < pxWidth; ncol++)
            {
                y = pYCbCr[nYCbCrId][0];
                u = pYCbCr[nYCbCrId][1] - 128;
                v = pYCbCr[nYCbCrId][2] - 128;
                pPictureMap[nRow][ncol].r = I32RGBToU8RGB((int32_t)(y + (1.402 * v)));
                pPictureMap[nRow][ncol].g = I32RGBToU8RGB((int32_t)(y - (0.344136 * u) - (0.714136 * v)));
                pPictureMap[nRow][ncol].b = I32RGBToU8RGB((int32_t)(y + (1.772 * u)));
                pPictureMap[nRow][ncol].a = 0xFF;
                nYCbCrId++;
            }
            /* 跳过边界 MCU 像素 */
            nYCbCrId += nSkip;
        }
    }
}
static void ConvertYCbCrToRGBFast(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    HANPPICTURE pPicture = jpegInfo->pictureData.pPictureInfo->pPicture[0];
    PPICTUREJPEGYCBCR pYCbCr = jpegInfo->pictureData.pYCbCrData;
    uint32_t pxWidth = pPicture->pxResolution.pxWidth;
    uint32_t pxHeight = pPicture->pxResolution.pxHeight;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    int32_t y;
    int32_t u;
    int32_t v;
    HANSIZE nYCbCrId = 0;
    uint32_t nSkip = jpegInfo->segment.segmentInfo.SOFn.pxMCUCnt.pxWidth * jpegInfo->segment.segmentInfo.SOFn.pxMCUSize.pxWidth - pxWidth;

    if (1 == jpegInfo->segment.segmentInfo.SOFn.nComponentCnt)
    {
        for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
        {
            for (uint32_t ncol = 0; ncol < pxWidth; ncol++)
            {
                pPictureMap[nRow][ncol].r = (uint8_t)pYCbCr[nYCbCrId][0];
                pPictureMap[nRow][ncol].g = (uint8_t)pYCbCr[nYCbCrId][0];
                pPictureMap[nRow][ncol].b = (uint8_t)pYCbCr[nYCbCrId][0];
                pPictureMap[nRow][ncol].a = 0xFF;
                nYCbCrId++;
            }
            /* 跳过边界 MCU 像素 */
            nYCbCrId += nSkip;
        }
    }
    else
    {
        for (uint32_t nRow = 0; nRow < pxHeight; nRow++)
        {
            for (uint32_t ncol = 0; ncol < pxWidth; ncol++)
            {
                y = pYCbCr[nYCbCrId][0];
                u = pYCbCr[nYCbCrId][1] - 128;
                v = pYCbCr[nYCbCrId][2] - 128;
                pPictureMap[nRow][ncol].r = I32RGBToU8RGB(y + ((INT_1_402 * v) >> 16));
                pPictureMap[nRow][ncol].g = I32RGBToU8RGB(y - ((INT_0_344136 * u) >> 16) - ((INT_0_714136 * v) >> 16));
                pPictureMap[nRow][ncol].b = I32RGBToU8RGB(y + ((INT_1_772 * u) >> 16));
                pPictureMap[nRow][ncol].a = 0xFF;
                nYCbCrId++;
            }
            /* 跳过边界 MCU 像素 */
            nYCbCrId += nSkip;
        }
    }
}
static inline uint8_t I32RGBToU8RGB(int32_t cRGB)
{
    uint8_t cRet;

    if (cRGB < 0) { cRet = 0; }
    else if (255 < cRGB) { cRet = 255; }
    else { cRet = cRGB; }

    return cRet;
}

static void JpegDecodeModeAuto(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PICTURERESOLUTION pxResolution = jpegInfo->segment.segmentInfo.SOFn.pxResolution;
    HANSIZE nPictureSize = pxResolution.pxWidth * pxResolution.pxHeight;
    
    if (nPictureSize < (PICTURE_JPEG_DECODE_PROCEDURE_MODE_WIDTH_THRESHOLD * PICTURE_JPEG_DECODE_PROCEDURE_MODE_WIDTH_THRESHOLD))
    {
        JpegDecodeModeSlow(jpegInfo);
    }
    else
    {
        JpegDecodeModeFast(jpegInfo);
    }
}
static void JpegDecodeModeSlow(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PictureJpegIDCT(jpegInfo->pictureData.pIDCTData, jpegInfo->pictureData.pDecodedData2D, jpegInfo->pictureData.nIDCTDataLen);
    MapJpegPixels(jpegInfo);
    ConvertYCbCrToRGB(jpegInfo);
}
static void JpegDecodeModeFast(PPICTUREJPEGWNDEXTRA jpegInfo)
{
    PictureJpegFastIDCT(jpegInfo->pictureData.pIDCTData, jpegInfo->pictureData.pDecodedData2D, jpegInfo->pictureData.nIDCTDataLen);
    MapJpegPixels(jpegInfo);
    ConvertYCbCrToRGBFast(jpegInfo);
}
#endif

#if 1 /* 编码 */
/* 编码参考解码 */
static void EncodeSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    switch (pEncode->jpegType) {
        case PICTURE_JPEG_TYPE_BASELINE: { EncodeBaselineSOS(pEncode, hFile); } break;
        case PICTURE_JPEG_TYPE_PROGRESSIVE: { EncodeProgressiveSOS(pEncode, hFile); } break;
        default: { } break;
    }
}
#if 1 /* 基线式 JPEG */
static void EncodeBaselineSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    uint8_t pMarker[2] = { 0xFF, 0xDA, };
    uint16_t nLen;
    uint8_t pHuffmanId[3] = { 0x00, 0x11, 0x11, };
    const PICTUREJPEGDCTMATRIX2D* pQT;
    PICTUREJPEGDCTMATRIX2D* pBlock;
    HANSIZE nBlockTotal;
    uint8_t* pData = pEncode->pData;
    HANSIZE nOffset = 0;

    memcpy(&pData[nOffset], pMarker, 2); nOffset += 4;
    memcpy(&pData[nOffset], &(pEncode->nComponent), 1); nOffset += 1;

    for (uint8_t iLoop = 0; iLoop < pEncode->nComponent; iLoop++)
    {
        pData[nOffset] = iLoop + 1; nOffset++;
        pData[nOffset] = pHuffmanId[iLoop]; nOffset++;
    }
    pData[nOffset] = 0; nOffset++; // Ss
    pData[nOffset] = 63; nOffset++; // Se
    pData[nOffset] = 0; nOffset++; // Ah-Al

    nLen = (uint16_t)(nOffset - 2);
    WriteJpegData2ByteMSB(&pData[2], nLen);
    
    WriteFile(hFile, pData, (DWORD)nOffset, NULL, NULL);

    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][0] = (pHuffmanId[0] >> 4) & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][1] = (pHuffmanId[1] >> 4) & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][2] = (pHuffmanId[2] >> 4) & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][0] = pHuffmanId[0] & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][1] = pHuffmanId[1] & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][2] = pHuffmanId[2] & 0x0F;

    for (uint8_t nComponentId = 0; nComponentId < pEncode->nComponent; nComponentId++)
    {
        pBlock = pEncode->pComponentTransform[nComponentId];
        nBlockTotal = pEncode->pComponentBlockTotal[nComponentId];
        pQT = &(pEncode->pQT->pQuantization[pEncode->nQTId[nComponentId]]);

        PictureJpegDCT(pBlock, pEncode->pComponentSample[nComponentId], nBlockTotal);
        EncodeSOSQuantization(pBlock, pQT, nBlockTotal);
    }
    EncodeSOSZigZag(pEncode);
    EncodeBaselineSOSHuffman(pEncode, hFile);
}
static void EncodeBaselineSOSHuffman(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    uint8_t nComponent = pEncode->nComponent;
    uint8_t* pMCUComponentBlockCnt = pEncode->pMCUComponentBlockCnt;
    PICTUREJPEGDCTMATRIX1D* pEncodeData = pEncode->pEncodeData;
    PCPICTUREJPEGSEGMENTDHTINFO pHTDC;
    PCPICTUREJPEGSEGMENTDHTINFO pHTAC;
    HANSIZE nBlockId;
    uint16_t cCode;
    uint8_t nRRRR;
    uint8_t nSSSS;
    uint8_t nRRRRSSSS;
    uint8_t nSpectral;

    pEncode->wbBits = JpegInitWriteBits(pEncode->pData, PICTURE_JPEG_SAVE_ENCODE_BUF_SIZE, hFile);

    nBlockId = 0;
    for (HANSIZE nMCUId = 0; nMCUId < pEncode->nMCUTotal; nMCUId++)
    {
        for (uint8_t nComponentId = 0; nComponentId < nComponent; nComponentId++)
        {
            pHTDC = EncodeSOSGetComponentHuffmanTable(pEncode, nComponentId, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC);
            pHTAC = EncodeSOSGetComponentHuffmanTable(pEncode, nComponentId, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC);
            for (uint8_t nMCUComponentBlockId = 0; nMCUComponentBlockId < pMCUComponentBlockCnt[nComponentId]; nMCUComponentBlockId++)
            {
                /* DC */
                EncodeVLI(pEncodeData[nBlockId][0], &cCode, &nSSSS);
                JpegWriteBits(&(pEncode->wbBits), pHTDC->pCode[nSSSS], pHTDC->pLen[nSSSS]);
                JpegWriteBits(&(pEncode->wbBits), cCode, nSSSS);
                /* AC */
                nRRRR = 0;
                nSpectral = 0;
                for (uint8_t iLoop = 1; iLoop < 64; iLoop++)
                {
                    if (0 == pEncodeData[nBlockId][iLoop])
                    {
                        nRRRR++;
                    }
                    else
                    {
                        /* 先写 ZRL */
                        while (16 <= nRRRR)
                        {
                            JpegWriteBits(&(pEncode->wbBits), pHTAC->pCode[0xF0], pHTAC->pLen[0xF0]);
                            nRRRR -= 16;
                        }
                        /* 再写 Run-Size */
                        EncodeVLI(pEncodeData[nBlockId][iLoop], &cCode, &nSSSS);
                        nRRRRSSSS = (nRRRR << 4) + nSSSS;
                        JpegWriteBits(&(pEncode->wbBits), pHTAC->pCode[nRRRRSSSS], pHTAC->pLen[nRRRRSSSS]);
                        JpegWriteBits(&(pEncode->wbBits), cCode, nSSSS);
                        
                        nRRRR = 0;
                        nSpectral = iLoop;
                    }
                }
                if (nSpectral < 63) { JpegWriteBits(&(pEncode->wbBits), pHTAC->pCode[0x00], pHTAC->pLen[0x00]); } // 频谱未记录到最后，说明是 0 结尾，写 EOB
                nBlockId++;
            }
        }
    }
    JpegFillBits(&(pEncode->wbBits));
}
#endif
#if 1 /* 渐进式 JPEG */
static void EncodeProgressiveSOS(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    const PICTUREJPEGDCTMATRIX2D* pQT;
    PICTUREJPEGDCTMATRIX2D* pBlock;
    HANSIZE nBlockTotal;

    for (uint8_t nComponentId = 0; nComponentId < pEncode->nComponent; nComponentId++)
    {
        pBlock = pEncode->pComponentTransform[nComponentId];
        nBlockTotal = pEncode->pComponentBlockTotal[nComponentId];
        pQT = &(pEncode->pQT->pQuantization[pEncode->nQTId[nComponentId]]);

        PictureJpegDCT(pBlock, pEncode->pComponentSample[nComponentId], nBlockTotal);
        EncodeSOSQuantization(pBlock, pQT, nBlockTotal);
    }
    /* 二维转一维 */
    EncodeSOSZigZag(pEncode);

    EncodeProgressiveSOSDC(pEncode, hFile);
    EncodeProgressiveSOSAC(pEncode, hFile, 0, 1, 5);
    EncodeProgressiveSOSAC(pEncode, hFile, 1, 1, 5);
    EncodeProgressiveSOSAC(pEncode, hFile, 2, 1, 5);
    EncodeProgressiveSOSAC(pEncode, hFile, 0, 6, 63);
    EncodeProgressiveSOSAC(pEncode, hFile, 1, 6, 63);
    EncodeProgressiveSOSAC(pEncode, hFile, 2, 6, 63);
}
static void EncodeProgressiveSOSDC(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile)
{
    uint8_t pMarker[2] = { 0xFF, 0xDA, };
    uint16_t nLen;
    uint8_t pHuffmanId[3] = { 0x00, 0x10, 0x10, };
    uint8_t nComponent = pEncode->nComponent;
    uint8_t* pData = pEncode->pData;
    uint8_t* pMCUComponentBlockCnt = pEncode->pMCUComponentBlockCnt;
    PICTUREJPEGDCTMATRIX1D* pEncodeData = pEncode->pEncodeData;
    PCPICTUREJPEGSEGMENTDHTINFO pHTDC;
    HANSIZE nOffset = 0;
    uint16_t cCode;
    uint8_t nSSSS;
    HANSIZE nBlockId;

    pEncode->wbBits = JpegInitWriteBits(pEncode->pData, PICTURE_JPEG_SAVE_ENCODE_BUF_SIZE, hFile);

    memcpy(&pData[nOffset], pMarker, 2); nOffset += 4;
    memcpy(&pData[nOffset], &(pEncode->nComponent), 1); nOffset += 1;

    for (uint8_t iLoop = 0; iLoop < pEncode->nComponent; iLoop++)
    {
        pData[nOffset] = iLoop + 1; nOffset++;
        pData[nOffset] = pHuffmanId[iLoop]; nOffset++;
    }
    pData[nOffset] = 0; nOffset++; // Ss
    pData[nOffset] = 0; nOffset++; // Se
    pData[nOffset] = 0; nOffset++; // Ah-Al

    nLen = (uint16_t)(nOffset - 2);
    WriteJpegData2ByteMSB(&pData[2], nLen);
    
    WriteFile(hFile, pData, (DWORD)nOffset, NULL, NULL);

    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][0] = (pHuffmanId[0] >> 4) & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][1] = (pHuffmanId[1] >> 4) & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC][2] = (pHuffmanId[2] >> 4) & 0x0F;

    nBlockId = 0;
    for (HANSIZE nMCUId = 0; nMCUId < pEncode->nMCUTotal; nMCUId++)
    {
        for (uint8_t nComponentId = 0; nComponentId < nComponent; nComponentId++)
        {
            pHTDC = EncodeSOSGetComponentHuffmanTable(pEncode, nComponentId, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_DC);
            for (uint8_t nMCUComponentBlockId = 0; nMCUComponentBlockId < pMCUComponentBlockCnt[nComponentId]; nMCUComponentBlockId++)
            {
                /* DC */
                EncodeVLI(pEncodeData[nBlockId][0], &cCode, &nSSSS);
                JpegWriteBits(&(pEncode->wbBits), pHTDC->pCode[nSSSS], pHTDC->pLen[nSSSS]);
                JpegWriteBits(&(pEncode->wbBits), cCode, nSSSS);
                nBlockId++;
            }
        }
    }
    JpegFillBits(&(pEncode->wbBits));
}
static void EncodeProgressiveSOSAC(PPICTUREJPEGENCODEMACHINE pEncode, HANDLE hFile, uint8_t nTargetComponent, uint8_t nSs, uint8_t nSe)
{
    uint8_t pMarker[2] = { 0xFF, 0xDA, };
    uint16_t nLen;
    uint8_t pHuffmanId[3] = { 0x00, 0x10, 0x10, };
    uint8_t* pData = pEncode->pData;
    const uint8_t* pSamplingCoe = pEncode->pSamplingCoe[nTargetComponent];
    PPICTURERESOLUTION pResolution = &(pEncode->pPicture->pxResolution);
    PPICTURERESOLUTION pMCUCnt = &(pEncode->pxMCUCnt);
    PPICTURERESOLUTION pMCUSize = &(pEncode->pxMCUSize);
    uint8_t nMCUComponentBlockOffset = pEncode->pMCUComponentBlockOffset[nTargetComponent];
    uint8_t nMCUComponentBlockTotal = pEncode->nMCUComponentBlockTotal;
    PICTUREJPEGDCTMATRIX1D* pEncodeData = pEncode->pEncodeData;
    PICTURERESOLUTION pxBlockSize;
    PICTURERESOLUTION pxBlockCnt;
    PICTURERESOLUTION pxMCUPos;
    PICTURERESOLUTION pxBlockPos;
    PCPICTUREJPEGSEGMENTDHTINFO pHTAC;
    HANSIZE nOffset = 0;
    uint16_t cCode;
    uint8_t nRRRR;
    uint8_t nSSSS;
    uint8_t nRRRRSSSS;
    uint8_t nSpectral;
    uint16_t nEOBRUN;
    HANSIZE nMCUId;
    HANSIZE nBlockOffset;
    HANSIZE nBlockId;

    pEncode->wbBits = JpegInitWriteBits(pEncode->pData, PICTURE_JPEG_SAVE_ENCODE_BUF_SIZE, hFile);

    memcpy(&pData[nOffset], pMarker, 2); nOffset += 4;
    
    pData[nOffset] = 1; nOffset++;
    pData[nOffset] = nTargetComponent + 1; nOffset++;
    pData[nOffset] = pHuffmanId[nTargetComponent]; nOffset++;

    pData[nOffset] = nSs; nOffset++; // Ss
    pData[nOffset] = nSe; nOffset++; // Se
    pData[nOffset] = 0; nOffset++; // Ah-Al

    nLen = (uint16_t)(nOffset - 2);
    WriteJpegData2ByteMSB(&pData[2], nLen);
    
    WriteFile(hFile, pData, (DWORD)nOffset, NULL, NULL);

    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][0] = pHuffmanId[0] & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][1] = pHuffmanId[1] & 0x0F;
    pEncode->nHTId[PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC][2] = pHuffmanId[2] & 0x0F;
    pHTAC = EncodeSOSGetComponentHuffmanTable(pEncode, nTargetComponent, PICTURE_JPEG_SEGMENT_DHT_TABLE_TYPE_AC);

    pxBlockSize.pxWidth = pMCUSize->pxWidth / pSamplingCoe[0];
    pxBlockSize.pxHeight = pMCUSize->pxHeight / pSamplingCoe[1];
    pxBlockCnt.pxWidth = (pResolution->pxWidth + pxBlockSize.pxWidth - 1) / pxBlockSize.pxWidth;
    pxBlockCnt.pxHeight = (pResolution->pxHeight + pxBlockSize.pxHeight - 1) / pxBlockSize.pxHeight;

    nEOBRUN = 0;
    nBlockId = 0;
    for (uint32_t nBlockRow = 0; nBlockRow < pxBlockCnt.pxHeight; nBlockRow++)
    {
        pxMCUPos.pxHeight = nBlockRow / pSamplingCoe[1];    // MCU 位于第几行
        pxBlockPos.pxHeight = nBlockRow % pSamplingCoe[1];  // Block 位于 MCU 中的第几行
        for (uint32_t nBlockCol = 0; nBlockCol < pxBlockCnt.pxWidth; nBlockCol++)
        {
            pxMCUPos.pxWidth = nBlockCol / pSamplingCoe[0];     // MCU 位于第几列
            pxBlockPos.pxWidth = nBlockCol % pSamplingCoe[0];   // Block 位于 MCU 中的第几列
            nMCUId = (pxMCUPos.pxHeight * pMCUCnt->pxWidth) + pxMCUPos.pxWidth;
            nBlockOffset = (pxBlockPos.pxHeight * pSamplingCoe[0]) + pxBlockPos.pxWidth;
            nBlockId = nMCUId * nMCUComponentBlockTotal + nMCUComponentBlockOffset + nBlockOffset;

            nRRRR = 0;
            nSpectral = nSs;
            for (uint8_t iLoop = nSs; iLoop <= nSe; iLoop++)
            {
                if (0 == pEncodeData[nBlockId][iLoop])
                {
                    nRRRR++;
                }
                else
                {
                    /* 先写 EOBRUN，如果没有出现过 EOB，会在函数内跳过 */
                    EncodeProgressiveSOWriteEOBRUN(pEncode, nEOBRUN, pHTAC);
                    /* 再写 ZRL */
                    while (16 <= nRRRR)
                    {
                        JpegWriteBits(&(pEncode->wbBits), pHTAC->pCode[0xF0], pHTAC->pLen[0xF0]);
                        nRRRR -= 16;
                    }
                    /* 再写 Run-Size */
                    EncodeVLI(pEncodeData[nBlockId][iLoop], &cCode, &nSSSS);
                    nRRRRSSSS = (nRRRR << 4) + nSSSS;
                    JpegWriteBits(&(pEncode->wbBits), pHTAC->pCode[nRRRRSSSS], pHTAC->pLen[nRRRRSSSS]);
                    JpegWriteBits(&(pEncode->wbBits), cCode, nSSSS);
                    
                    nRRRR = 0;
                    nSpectral = iLoop;
                    nEOBRUN = 0;
                }
            }
            if (nSpectral < nSe)
            {
                nEOBRUN++;
                if (nEOBRUN == (uint16_t)0x7FFF)
                {
                    EncodeProgressiveSOWriteEOBRUN(pEncode, nEOBRUN, pHTAC);
                    nEOBRUN -= 0x7FFF;
                }
            }
            nBlockId++;
        }
    }
    EncodeProgressiveSOWriteEOBRUN(pEncode, nEOBRUN, pHTAC);
    JpegFillBits(&(pEncode->wbBits));
}
static inline void EncodeProgressiveSOWriteEOBRUN(PPICTUREJPEGENCODEMACHINE pEncode, uint16_t nEOBRUN, PCPICTUREJPEGSEGMENTDHTINFO pHTAC)
{
    uint8_t nSSSS;
    uint8_t nI;
    uint16_t cCode;

    if (0 < nEOBRUN)
    {
        EncodeVLI(nEOBRUN, &cCode, &nSSSS);
        nSSSS--;
        nI = nSSSS << 4;
        cCode -= 1 << nSSSS;
        JpegWriteBits(&(pEncode->wbBits), pHTAC->pCode[nI], pHTAC->pLen[nI]);
        JpegWriteBits(&(pEncode->wbBits), cCode, nSSSS);
    }
}
#endif

static void EncodeSOSQuantization(PICTUREJPEGDCTMATRIX2D* pBlock, const PICTUREJPEGDCTMATRIX2D* pQT, HANSIZE nLen)
{
    PICTUREJPEGNUMBER nPrev = 0;
    PICTUREJPEGNUMBER nCur;

    for (HANSIZE iLoop = 0; iLoop < nLen; iLoop++)
    {
        for (uint8_t nRow = 0; nRow < 8; nRow++)
        {
            for (uint8_t nCol = 0; nCol < 8; nCol++)
            {
                pBlock[iLoop][nRow][nCol] /= (*pQT)[nRow][nCol];
            }
        }
        nCur = pBlock[iLoop][0][0];
        pBlock[iLoop][0][0] -= nPrev;
        nPrev = nCur;
    }
}
static void EncodeSOSZigZag(PPICTUREJPEGENCODEMACHINE pEncode)
{
    PICTUREJPEGDCTMATRIX2D** pComponent = pEncode->pComponentTransform;
    PICTUREJPEGDCTMATRIX1D* pEncodeData = pEncode->pEncodeData;
    HANSIZE nMCUTotal = pEncode->nMCUTotal;
    uint8_t nComponent = pEncode->nComponent;
    uint8_t* pMCUComponentBlockCnt = pEncode->pMCUComponentBlockCnt;
    uint8_t nZZOffset;
    HANSIZE pBlockOffset[PICTURE_JPEG_SEGMENT_COMPONENT_MAX];
    HANSIZE nBlockOffset;
    HANSIZE nOffset;

    memset(pBlockOffset, 0, sizeof(pBlockOffset));
    nOffset = 0;
    for (HANSIZE nMCUId = 0; nMCUId < nMCUTotal; nMCUId++)
    {
        for (uint8_t nComponentId = 0; nComponentId < nComponent; nComponentId++)
        {
            for (uint8_t nBlockId = 0; nBlockId < pMCUComponentBlockCnt[nComponentId]; nBlockId++)
            {
                nBlockOffset = pBlockOffset[nComponentId];
                for (uint8_t nRow = 0; nRow < 8; nRow++)
                {
                    for (uint8_t nCol = 0; nCol < 8; nCol++)
                    {
                        nZZOffset = sg_pZigZagOrderTable[nRow][nCol];
                        pEncodeData[nOffset][nZZOffset] = pComponent[nComponentId][nBlockOffset][nRow][nCol];
                    }
                }
                pBlockOffset[nComponentId]++;
                nOffset++;
            }
        }
    }
}
static inline PCPICTUREJPEGSEGMENTDHTINFO EncodeSOSGetComponentHuffmanTable(PPICTUREJPEGENCODEMACHINE pEncode, uint8_t nComponent, PICTUREJPEGSEGMENTDHTTABLETYPE eType)
{
    PCPICTUREJPEGSEGMENTDHTINFO pRet;
    uint8_t nHTId;

    nHTId = pEncode->nHTId[eType][nComponent];
    pRet = pEncode->pHT->pHuffman[eType][nHTId];

    return pRet;
}

static PICTUREJPEGWRITEBITS JpegInitWriteBits(uint8_t* pData, HANSIZE nLen, HANDLE hFile)
{
    PICTUREJPEGWRITEBITS wbRet = {
        .hFile = hFile,
        .pData = pData,
        .nDataLen = nLen,
        .cData = 0,
        .iByte = 0,
        .iBit = 0,
    };

    return wbRet;
}
static inline void JpegWriteBits(PPICTUREJPEGWRITEBITS pWriteBits, uint16_t cData, uint8_t nLen)
{
    uint8_t nTmp = nLen;
    uint8_t nFreeLen;
    uint16_t cMask;

    while (0 < nTmp)
    {
        nFreeLen = 8 - pWriteBits->iBit;
        if (nTmp <= nFreeLen)
        {
            pWriteBits->cData += cData << (nFreeLen - nTmp);
            pWriteBits->iBit += nTmp;
            nTmp = 0;
        }
        else
        {
            cMask = (1 << nFreeLen) - 1;
            pWriteBits->cData += (cData >> (nTmp - nFreeLen)) & cMask;
            pWriteBits->iBit = 8;
            nTmp -= nFreeLen;
        }

        if (8 == pWriteBits->iBit)
        {
            pWriteBits->pData[pWriteBits->iByte] = pWriteBits->cData;
            pWriteBits->iByte++;
            if (0xFF == pWriteBits->cData)
            {
                pWriteBits->pData[pWriteBits->iByte] = 0x00;
                pWriteBits->iByte++;
            }
            pWriteBits->iBit = 0;
            pWriteBits->cData = 0;

            if ((pWriteBits->nDataLen - 10) < pWriteBits->iByte)
            {
                WriteFile(pWriteBits->hFile, pWriteBits->pData, (DWORD)(pWriteBits->iByte), NULL, NULL);
                pWriteBits->iByte = 0;
            }
        }
    }
}
static void JpegFillBits(PPICTUREJPEGWRITEBITS pWriteBits)
{
    if (0 < pWriteBits->iBit)
    {
        pWriteBits->cData += (1 << (8 - pWriteBits->iBit)) - 1;
        pWriteBits->pData[pWriteBits->iByte] = pWriteBits->cData;
        pWriteBits->iByte++;
    }
    if (0 < pWriteBits->iByte)
    {
        WriteFile(pWriteBits->hFile, pWriteBits->pData, (DWORD)(pWriteBits->iByte), NULL, NULL);
    }
}
static inline void EncodeVLI(PICTUREJPEGNUMBER nValue, uint16_t* pCode, uint8_t* nLen)
{
    uint16_t nTmp;

    if (nValue < 0)
    {
        nTmp = (uint16_t)(-nValue);
        *pCode = ~nTmp;
    }
    else
    {
        nTmp = (uint16_t)nValue;
        *pCode = nTmp;
    }

    *nLen = 0;
    while (nTmp > 0)
    {
        nTmp >>= 1;
        (*nLen)++;
    }
    *pCode &= (1 << (*nLen)) - 1;
}
#endif
