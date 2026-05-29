#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PicturePng.h"
#include "HAN_PicturePngChunk.h"
#include "HAN_PicturePngPaint.h"
#include "..\..\..\..\Lib\zlib-1.3.1\zlib.h"
#include "..\..\..\..\HAN_Lib\HAN_windows.h"
#include "..\..\..\..\HAN_Lib\HAN_CRC.h"
#include "..\..\HAN_PictureDef.h"
#include "..\..\HAN_PictureLib.h"
#include "..\EditTool\HAN_PictureEditTool.h"

#define HAN_PICTURE_PNG_COLOR_TYPE_GRAY_SCALE       0
#define HAN_PICTURE_PNG_COLOR_TYPE_TRUE_COLOR       2
#define HAN_PICTURE_PNG_COLOR_TYPE_INDEX_COLOR      3
#define HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_GRAY_SCALE 4
#define HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_TRUE_COLOR 6

#define PICTURE_PNG_ADAM7_SIZE                      7

typedef uint16_t PNGCOLOR;

typedef enum {
    PICTURE_PNG_CHUNK_LIST_HEADER_TYPE,
    PICTURE_PNG_CHUNK_LIST_HEADER_LEN,
    PICTURE_PNG_CHUNK_LIST_HEADER_DATA,
    PICTURE_PNG_CHUNK_LIST_HEADER_CRC,
    PICTURE_PNG_CHUNK_LIST_HEADER_CNT,
} PICTUREPNGCHUNKLISTHEADER;

typedef enum {
    PICTURE_PNG_CHUNK_INFO_HEADER_FIELD,
    PICTURE_PNG_CHUNK_INFO_HEADER_VALUE,
    PICTURE_PNG_CHUNK_INFO_HEADER_CNT,
} PICTUREPNGCHUNKINFOHEADER;

typedef struct tagPICTUREPNGCHUNKHEADERINFO {
    HANPCSTR                        pName;
    HANINT                          nWidth;
} PICTUREPNGCHUNKHEADERINFO;

typedef struct tagPICTUREPNGPROCESSPARAM {
    const uint8_t*                  pData;
    uint32_t                        nLen;
    HWND                            hListViewReport;
    uint32_t                        pxWidth;
    uint32_t                        pxHeight;
} PICTUREPNGPROCESSPARAM, * PPICTUREPNGPROCESSPARAM;

typedef struct tagPICTUREPNGENCODEPARAM {
    uint8_t* pColor;
    uint8_t* pCompressData;
    HANSIZE nColorSize;
    HANSIZE nCompressSize;
    uint8_t pBuf[];
} PICTUREPNGENCODEPARAM, * PPICTUREPNGENCODEPARAM;

typedef struct tagPICTUREPNGWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HWND                            hShow;
    HWND                            hEditTool;
    PICTURECREATEPARAM              paramPicture;
    struct {
        HWND                        hList;
        HWND                        hInfo;
        PICTUREPNGCHUNKINFO         chunkInfo;
        struct {
            HANSIZE                 nCnt;
            PPICTUREPNGCHUNK        pChunkList;
        } map;
    } chunk;
    struct {
        PICTUREPIXELINFO            pixelInfo;
        uint8_t                     cColorType;         /* 颜色类型，如灰度图、真彩色图等 */
        uint8_t*                    pIDATData;          /* 存放所有 IDAT 数据块拼接后的内容（未滤波） */
        SIZE_T                      nIDATDataLen;       /* 所有 IDAT 数据块的总长 */
        uint8_t*                    pColorData;         /* 存放颜色数据，即解压（滤波前）并进行滤波（覆盖到滤波前的数据上）后的数据 */
        SIZE_T                      nColorDataLen;      /* 颜色数据的长度 */
        HANPPICTUREINFO             pPictureInfo;       /* 指向最终的图像信息 */
    } pictureData;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
    uint8_t                         pBuf[];
} PICTUREPNGWNDEXTRA, * PPICTUREPNGWNDEXTRA;

typedef struct tagPICTUREPNGCHUNKTYPEINFO {
    HANPSTR                         pCode;
    BOOL                            (*UpdateChunkInfo)(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
    void                            (*UpdateChunkInfoWindow)(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
} PICTUREPNGCHUNKTYPEINFO;

static LRESULT CALLBACK PicturePngWndProc(HWND hPicturePng, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPicturePng, LPARAM lParam);
static void SizeCallback(HWND hPicturePng, PPICTUREPNGWNDEXTRA pngInfo);
static LRESULT NotifyCallback(PPICTUREPNGWNDEXTRA pngInfo, NMHDR* pNotify);
static void DestroyCallback(PPICTUREPNGWNDEXTRA pngInfo);
static LRESULT GetSaveParamCallback(PPICTUREPNGWNDEXTRA pngInfo, PPICTURESAVEPARAM pSaveParam);
static void InitPngChunkWindow(PPICTUREPNGWNDEXTRA pngInfo);
static HANINT PngProcess(PPICTUREPNGWNDEXTRA pngInfo);
static PPICTUREPNGWNDEXTRA ReallocPngInfoMemory(PPICTUREPNGWNDEXTRA pngInfo);
static SIZE_T GetUncompressBufferSize(PPICTUREPNGWNDEXTRA pngInfo);
static HANSIZE ReadPngChunk(const uint8_t* pData, HANSIZE nLen, PPICTUREPNGCHUNK pChunk);
static inline HANSIZE ChunkDataLenToChunkLen(uint32_t nChunkDataLen);
static PICTUREPNGCHUNKTYPE GetPngChunkType(HANPCSTR pType);
static void SetChunkMap(const uint8_t* pData, HANINT nChunkCnt, PPICTUREPNGWNDEXTRA pngInfo);
static void MergeIDATData(PPICTUREPNGWNDEXTRA pngInfo);
static void GetPngShowSize(PPICTUREPNGWNDEXTRA pngInfo, HANINT* pW, HANINT* pH);
static void PicturePngPrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen);
static LRESULT ChunkListNotifyCallback(PPICTUREPNGWNDEXTRA pngInfo, NMHDR* pNotify);
static void UpdateChunkInfoWindow(PPICTUREPNGWNDEXTRA pngInfo, HANINT nId);

static inline HANINT GetChunkListWindowWidth(void);
static inline HANINT GetChunkInfoWindowWidth(void);
static inline uint32_t ReadPngData4ByteMSB(const uint8_t pData[4]);
static inline uint16_t ReadPngData2ByteMSB(const uint8_t pData[2]);
static inline void WritePngData4ByteMSB(uint8_t* pData, uint32_t nData);
static inline void WritePngData2ByteMSB(uint8_t* pData, uint32_t nData);
static inline PNGCOLOR ReadPngColor(const uint8_t* pData, uint8_t nColorSize);
static inline void WritePngColor(uint8_t* pDest, PNGCOLOR cColor, uint8_t nColorSize);

static BOOL UpdateChunkInfo_Default(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_IHDR(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_gAMA(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_pHYs(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_bKGD(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_tIME(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_iCCP(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_cHRM(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_PLTE(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_tRNS(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);
static BOOL UpdateChunkInfo_IDAT(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo);

static void UpdateChunkInfoWindow_Default(const PCHAR pCode, HWND hListView);
static void UpdateChunkInfoWindow_IHDR(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_gAMA(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_pHYs(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_bKGD(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_tIME(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_iCCP(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_cHRM(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_PLTE(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_tRNS(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_IDAT(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_tEXT(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);
static void UpdateChunkInfoWindow_IEND(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView);

static BOOL DecodeIDAT(PPICTUREPNGWNDEXTRA pngInfo);
static BOOL PngFilterDecode(uint8_t* pData, PCPICTURERESOLUTION pResolution, PPICTUREPIXELINFO pPixelInfo);
static void GetAdam7Resolution(PPICTURERESOLUTION pDestResolution, PCPICTURERESOLUTION pSrcResolution, uint8_t nLevel);
static inline PNGCOLOR PngFilterPaeth(PNGCOLOR pxA, PNGCOLOR pxB, PNGCOLOR pxC);
static void PngArrangePixel(PPICTUREPNGWNDEXTRA pngInfo);
static void PngArrangePixelNoInterlace(PPICTUREPNGWNDEXTRA pngInfo);
static void PngArrangePixelInterlace(PPICTUREPNGWNDEXTRA pngInfo);
static void SetPictureRGBA(PPICTUREPNGWNDEXTRA pngInfo, PPICTURERGBA pRGBA, uint8_t* pData);

static void SaveOriPictureInfo(PPICTUREPNGWNDEXTRA pngInfo, HANDLE hFile);
static BOOL SavePngChunk_IHDR(HANPPICTURE pPicture, HANDLE hFile);
static BOOL SavePngChunk_IDAT(HANPPICTURE pPicture, HANDLE hHeap, HANDLE hFile);
static BOOL SavePngChunk_IEND(HANDLE hFile);

static void InitIDATColorData(uint8_t* pData, HANPCPICTURE pPicture);

static const uint8_t sg_pPngHeader[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, };
static const uint32_t sg_pxAdam7StartH[PICTURE_PNG_ADAM7_SIZE] = { 0, 4, 0, 2, 0, 1, 0 };
static const uint32_t sg_pxAdam7StartV[PICTURE_PNG_ADAM7_SIZE] = { 0, 0, 4, 0, 2, 0, 1 };
static const uint32_t sg_pxAdam7DistanceH[PICTURE_PNG_ADAM7_SIZE] = { 8, 8, 4, 4, 2, 2, 1 };
static const uint32_t sg_pxAdam7DistanceV[PICTURE_PNG_ADAM7_SIZE] = { 8, 8, 8, 4, 4, 2, 2 };
static const PICTUREPNGCHUNKHEADERINFO sg_pPngChunkListHeader[PICTURE_PNG_CHUNK_LIST_HEADER_CNT] = {
    [PICTURE_PNG_CHUNK_LIST_HEADER_TYPE] = {
        .pName = TEXT("类型"),
        .nWidth = 60,
    },
    [PICTURE_PNG_CHUNK_LIST_HEADER_LEN] = {
        .pName = TEXT("长度"),
        .nWidth = 130,
    },
    [PICTURE_PNG_CHUNK_LIST_HEADER_DATA] = {
        .pName = TEXT("数据"),
        .nWidth = 400,
    },
    [PICTURE_PNG_CHUNK_LIST_HEADER_CRC] = {
        .pName = TEXT("校验码"),
        .nWidth = 100,
    },
};
static const HANINT sg_pPngChunkInfoHeaderWidth[PICTURE_PNG_CHUNK_LIST_HEADER_CNT] = {
    [PICTURE_PNG_CHUNK_INFO_HEADER_FIELD] = 100,
    [PICTURE_PNG_CHUNK_INFO_HEADER_VALUE] = 300,
};
static const PICTUREPNGCHUNKTYPEINFO sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_CNT] = {
    [PICTURE_PNG_CHUNK_TYPE_IHDR] = {
        .pCode = TEXT("IHDR"),
        .UpdateChunkInfo = UpdateChunkInfo_IHDR,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_IHDR,
    },
    [PICTURE_PNG_CHUNK_TYPE_gAMA] = {
        .pCode = TEXT("gAMA"),
        .UpdateChunkInfo = UpdateChunkInfo_gAMA,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_gAMA,
    },
    [PICTURE_PNG_CHUNK_TYPE_pHYs] = {
        .pCode = TEXT("pHYs"),
        .UpdateChunkInfo = UpdateChunkInfo_pHYs,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_pHYs,
    },
    [PICTURE_PNG_CHUNK_TYPE_bKGD] = {
        .pCode = TEXT("bKGD"),
        .UpdateChunkInfo = UpdateChunkInfo_bKGD,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_bKGD,
    },
    [PICTURE_PNG_CHUNK_TYPE_tIME] = {
        .pCode = TEXT("tIME"),
        .UpdateChunkInfo = UpdateChunkInfo_tIME,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_tIME,
    },
    [PICTURE_PNG_CHUNK_TYPE_iCCP] = {
        .pCode = TEXT("iCCP"),
        .UpdateChunkInfo = UpdateChunkInfo_iCCP,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_iCCP,
    },
    [PICTURE_PNG_CHUNK_TYPE_cHRM] = {
        .pCode = TEXT("cHRM"),
        .UpdateChunkInfo = UpdateChunkInfo_cHRM,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_cHRM,
    },
    [PICTURE_PNG_CHUNK_TYPE_PLTE] = {
        .pCode = TEXT("PLTE"),
        .UpdateChunkInfo = UpdateChunkInfo_PLTE,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_PLTE,
    },
    [PICTURE_PNG_CHUNK_TYPE_tRNS] = {
        .pCode = TEXT("tRNS"),
        .UpdateChunkInfo = UpdateChunkInfo_tRNS,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_tRNS,
    },
    [PICTURE_PNG_CHUNK_TYPE_IDAT] = {
        .pCode = TEXT("IDAT"),
        .UpdateChunkInfo = UpdateChunkInfo_IDAT,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_IDAT,
    },
    [PICTURE_PNG_CHUNK_TYPE_tEXT] = {
        .pCode = TEXT("tEXT"),
        .UpdateChunkInfo = UpdateChunkInfo_Default,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_tEXT,
    },
    [PICTURE_PNG_CHUNK_TYPE_IEND] = {
        .pCode = TEXT("IEND"),
        .UpdateChunkInfo = UpdateChunkInfo_Default,
        .UpdateChunkInfoWindow = UpdateChunkInfoWindow_IEND,
    },
};

BOOL CheckPngType(const uint8_t* pData, HANSIZE nLen)
{
    BOOL bRet;

    if (nLen < sizeof(sg_pPngHeader)) { bRet = FALSE; }
    else if (0 != memcmp(pData, sg_pPngHeader, sizeof(sg_pPngHeader))) { bRet = FALSE; }
    else { bRet = TRUE; }

    return bRet;
}

void RegisterHANPicturePng(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PicturePngWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREPNGWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)GetStockObject(PICTURE_BACKGROUND_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_PNG_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
    
    RegisterHANPicturePngPaint(hInst);
}

void SavePicturePng(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam)
{
    HANDLE hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    HANPPICTURE pPicture = pSaveParam->pPicture;

    if (INVALID_HANDLE_VALUE != hFile)
    {
        if (!HAN_strcmp(pSaveParam->pOpenClassName, pSaveParam->pSaveClassName))
        {
            SaveOriPictureInfo((PPICTUREPNGWNDEXTRA)(pSaveParam->pParam), hFile);
        }
        else
        {
            WriteFile(hFile, sg_pPngHeader, sizeof(sg_pPngHeader), NULL, NULL);
            SavePngChunk_IHDR(pPicture, hFile);
        }
        SavePngChunk_IDAT(pPicture, pSaveParam->hHeap, hFile);
        SavePngChunk_IEND(hFile);
        CloseHandle(hFile);
    }
}

static LRESULT CALLBACK PicturePngWndProc(HWND hPicturePng, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREPNGWNDEXTRA pngInfo = (PPICTUREPNGWNDEXTRA)GetWindowLongPtr(hPicturePng, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPicturePng, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(hPicturePng, pngInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_NOTIFY: {
            lWndProcRet = NotifyCallback(pngInfo, (NMHDR*)lParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(pngInfo);
            lWndProcRet = DefWindowProc(hPicturePng, message, wParam, lParam);
        } break;

        case PCTM_GETSAVEPARAM: {
            lWndProcRet = GetSaveParamCallback(pngInfo, (PPICTURESAVEPARAM)lParam);
        } break;
        case PCTM_ZOOM: {
            (void)SendMessage(pngInfo->hEditTool, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPicturePng, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPicturePng, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREPNGWNDEXTRA pngInfo;
    PPICTUREPNGWNDEXTRA pngTempInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PPICTURECREATEPARAM pPictureCreateParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    HANINT nChunkCnt;

    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_WINDOW_DY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        pngInfo = (PPICTUREPNGWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREPNGWNDEXTRA));
        if (NULL == pngInfo) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPicturePng, 0, (LONG_PTR)pngInfo);

        pngInfo->hHeap = hHeap;
        pngInfo->hInst = hInst;
        pngInfo->hSelf = hPicturePng;
        memset(&(pngInfo->chunk.chunkInfo), 0, sizeof(pngInfo->chunk.chunkInfo));

        GetClientRect(hPicturePng, &rcClientSize);
        nWinH = PICTURE_PNG_INFO_HEIGHT;

        pngInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        pngInfo->hFont.hSys = CreateFontIndirect(&g_lfInfoFont);

        nWinW = GetChunkListWindowWidth();
        pngInfo->chunk.hList = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPicturePng, (HMENU)WID_PICTURE_PNG_CHUNK_LIST, hInst, NULL);
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinW = GetChunkInfoWindowWidth();
        pngInfo->chunk.hInfo = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPicturePng, (HMENU)WID_PICTURE_PNG_CHUNK_INFO, hInst, NULL);

        SendMessage(pngInfo->chunk.hList, WM_SETFONT, (WPARAM)(pngInfo->hFont.hHex), (LPARAM)TRUE);
        SendMessage(pngInfo->chunk.hInfo, WM_SETFONT, (WPARAM)(pngInfo->hFont.hSys), (LPARAM)TRUE);

        ListView_SetExtendedListViewStyle(pngInfo->chunk.hList, LVS_EX_FULLROWSELECT);
        ListView_SetExtendedListViewStyle(pngInfo->chunk.hInfo, LVS_EX_FULLROWSELECT);
        
        InitPngChunkWindow(pngInfo);
    }
    /* 解码文件的数据段，填充信息 */
    if (-1 != lWndProcRet)
    {
        pngInfo->paramPicture = *pPictureCreateParam;
        nChunkCnt = PngProcess(pngInfo);
        pngInfo->chunk.map.nCnt = nChunkCnt;
        if (0 == nChunkCnt) { lWndProcRet = -1; }
    }
    /* 重新分配 pngInfo 内存 */
    if (-1 != lWndProcRet)
    {
        pngTempInfo = ReallocPngInfoMemory(pngInfo);
        if (NULL != pngTempInfo)
        {
            pngInfo = pngTempInfo;
            SetWindowLongPtr(hPicturePng, 0, (LONG_PTR)pngInfo);
        }
        else { lWndProcRet = -1; }
    }
    /* 图片解码 */
    if (-1 != lWndProcRet)
    {
        BOOL bDecodeRet = TRUE;

        SetChunkMap(pPictureCreateParam->pData, nChunkCnt, pngInfo);
        MergeIDATData(pngInfo);
        bDecodeRet = DecodeIDAT(pngInfo);
        
        /* 绘制预览图 */
        nWinX += nWinW + PICTURE_WINDOW_DX;
        GetPngShowSize(pngInfo, &nWinW, &nWinH);
        pngInfo->hShow = CreateWindow(HAN_PICTURE_PNG_PAINT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER, nWinX, nWinY, nWinW, nWinH,
            hPicturePng, (HMENU)WID_PICTURE_PNG_SHOW, hInst, pngInfo->pictureData.pPictureInfo->pPicture[0]
        );
        nWinX = PICTURE_WINDOW_DX;
        nWinY += PICTURE_PNG_INFO_HEIGHT + PICTURE_WINDOW_DY;
        nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
        nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;
        pngInfo->hEditTool = CreateWindow(HAN_PICTURE_EDIT_TOOL_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, nWinW, nWinH,
            hPicturePng, (HMENU)WID_PICTURE_EDIT_TOOL, hInst, pngInfo->pictureData.pPictureInfo
        );
        if ((FALSE == bDecodeRet) || (NULL == pngInfo->hShow) || (NULL == pngInfo->hEditTool)) { lWndProcRet = -1; }
    }

    return lWndProcRet;
}
static void SizeCallback(HWND hPicturePng, PPICTUREPNGWNDEXTRA pngInfo)
{
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_PNG_INFO_HEIGHT + (PICTURE_WINDOW_DY * 2);
    HANINT nWinW;
    HANINT nWinH;
    RECT rcClientSize;
    
    GetClientRect(hPicturePng, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
    nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;

    MoveWindow(pngInfo->hEditTool, nWinX, nWinY, nWinW, nWinH, TRUE);
}
static LRESULT NotifyCallback(PPICTUREPNGWNDEXTRA pngInfo, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->idFrom) {
        case WID_PICTURE_PNG_CHUNK_LIST: {
            lWndProcRet = ChunkListNotifyCallback(pngInfo, pNotify);
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void DestroyCallback(PPICTUREPNGWNDEXTRA pngInfo)
{
    HANWinHeapFree(pngInfo->hHeap, 0, pngInfo);
}
static LRESULT GetSaveParamCallback(PPICTUREPNGWNDEXTRA pngInfo, PPICTURESAVEPARAM pSaveParam)
{
    HAN_strcpy(pSaveParam->pOpenClassName, HAN_PICTURE_PNG_CLASS);
    pSaveParam->pPicture = pngInfo->pictureData.pPictureInfo->pPicture[0];
    pSaveParam->pParam = pngInfo;

    return TRUE;
}
static void InitPngChunkWindow(PPICTUREPNGWNDEXTRA pngInfo)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };
    
    for (PICTUREPNGCHUNKLISTHEADER iLoop = 0; iLoop < PICTURE_PNG_CHUNK_LIST_HEADER_CNT; iLoop++)
    {
        lvTitle.iSubItem = iLoop;
        lvTitle.pszText = (HANPSTR)sg_pPngChunkListHeader[iLoop].pName;
        lvTitle.cx = sg_pPngChunkListHeader[iLoop].nWidth;
        lvTitle.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(pngInfo->chunk.hList, iLoop, &lvTitle);
    }
    for (PICTUREPNGCHUNKINFOHEADER iLoop = 0; iLoop < PICTURE_PNG_CHUNK_INFO_HEADER_CNT; iLoop++)
    {
        lvTitle.iSubItem = iLoop;
        lvTitle.pszText = TEXT("");
        lvTitle.cx = sg_pPngChunkInfoHeaderWidth[iLoop];
        lvTitle.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(pngInfo->chunk.hInfo, iLoop, &lvTitle);
    }
}
static HANINT PngProcess(PPICTUREPNGWNDEXTRA pngInfo)
{
    HANINT nRet = 0;
    const uint8_t* pData = pngInfo->paramPicture.pData;
    HANSIZE nLen = pngInfo->paramPicture.nLen;
    HWND hListViewReport = pngInfo->chunk.hList;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    HANSIZE nOffset = sizeof(sg_pPngHeader);
    PICTUREPNGCHUNK pngChunk;
    HANSIZE nChunkLen;
    PICTUREPNGCHUNKTYPE eType;
    LVITEM lvItem = { .mask = LVIF_TEXT, };

    while (nOffset < nLen)
    {
        nChunkLen = ReadPngChunk(&pData[nOffset], nLen - nOffset, &pngChunk);
        if (0 < nChunkLen)
        {
            lvItem.iItem = nRet;
            lvItem.iSubItem = 0;
            lvItem.pszText = pText;
            ListView_InsertItem(hListViewReport, &lvItem);

            lvItem.iSubItem = PICTURE_PNG_CHUNK_LIST_HEADER_TYPE;
            HAN_snprintf(pText, ArrLen(pText), TEXT("%c%c%c%c"), pngChunk.pTypeCode[0], pngChunk.pTypeCode[1], pngChunk.pTypeCode[2], pngChunk.pTypeCode[3]);
            ListView_SetItem(hListViewReport, &lvItem);
            eType = GetPngChunkType(pText);
            
            lvItem.iSubItem = PICTURE_PNG_CHUNK_LIST_HEADER_LEN;
            HAN_snprintf(pText, ArrLen(pText), TEXT("%u"), pngChunk.nLength);
            ListView_SetItem(hListViewReport, &lvItem);
            
            lvItem.iSubItem = PICTURE_PNG_CHUNK_LIST_HEADER_DATA;
            PicturePngPrintHexData(pText, ArrLen(pText), pngChunk.pData, pngChunk.nLength);
            ListView_SetItem(hListViewReport, &lvItem);
            
            lvItem.iSubItem = PICTURE_PNG_CHUNK_LIST_HEADER_CRC;
            HAN_snprintf(pText, ArrLen(pText), TEXT("%08X"), pngChunk.cCrc);
            ListView_SetItem(hListViewReport, &lvItem);

            if ((0 <= eType) && (eType < PICTURE_PNG_CHUNK_TYPE_CNT))
            {
                if (FALSE == sg_pPngChunkType[eType].UpdateChunkInfo(&pngChunk, pngInfo)) { nChunkLen = 0; }
            }

            nRet++;
            nOffset += nChunkLen;
        }

        if (0 == nChunkLen)
        {
            nRet = 0;
            break;
        }
    }
    
    return nRet;
}
static PPICTUREPNGWNDEXTRA ReallocPngInfoMemory(PPICTUREPNGWNDEXTRA pngInfo)
{
    PPICTUREPNGWNDEXTRA pRet;
    PICTURERESOLUTION pxResolution = pngInfo->chunk.chunkInfo.IHDR.pxResolution;
    HANSIZE nChunkCnt = pngInfo->chunk.map.nCnt;
    SIZE_T nOffset;
    SIZE_T nChunkSize;
    SIZE_T nIDATSize;
    SIZE_T nPaperSize;
    SIZE_T nPictureInfoSize;
    SIZE_T nPictureSize;

    nChunkSize = sizeof(PICTUREPNGCHUNK) * nChunkCnt;
    nIDATSize = pngInfo->pictureData.nIDATDataLen;
    nPaperSize = GetUncompressBufferSize(pngInfo);
    nPictureInfoSize = GetPictureInfoMemSize(1);
    nPictureSize = GetPictureMemSize(&pxResolution);
    pRet = (PPICTUREPNGWNDEXTRA)HANWinHeapAlloc(
        pngInfo->hHeap, pngInfo,
        sizeof(PICTUREPNGWNDEXTRA)
         + nChunkSize
         + nIDATSize
         + nPaperSize
         + nPictureInfoSize
         + nPictureSize
    );

    if (NULL != pRet)
    {
        nOffset = 0;
        pRet->chunk.map.pChunkList = (PPICTUREPNGCHUNK)&(pRet->pBuf[nOffset]); nOffset += nChunkSize;
        pRet->pictureData.pIDATData = (uint8_t*)&(pRet->pBuf[nOffset]); nOffset += nIDATSize;
        pRet->pictureData.nColorDataLen = nPaperSize;
        pRet->pictureData.pColorData = (uint8_t*)&(pRet->pBuf[nOffset]); nOffset += nPaperSize;
        pRet->pictureData.pPictureInfo = (HANPPICTUREINFO)&(pRet->pBuf[nOffset]); nOffset += nPictureInfoSize;
        pRet->pictureData.pPictureInfo->nCnt = 1;
        pRet->pictureData.pPictureInfo->pPicture[0] = (HANPPICTURE)&(pRet->pBuf[nOffset]); nOffset += nPictureSize;
        UpdatePictureMap(pRet->pictureData.pPictureInfo->pPicture[0], &pxResolution);
    }

    return pRet;
}
static SIZE_T GetUncompressBufferSize(PPICTUREPNGWNDEXTRA pngInfo)
{
    SIZE_T nRet;
    uint8_t nPixelSize = pngInfo->pictureData.pixelInfo.nPixelSize;
    PCPICTURERESOLUTION pResolution = &(pngInfo->chunk.chunkInfo.IHDR.pxResolution);
    PICTURERESOLUTION pxResolution;

    switch (pngInfo->chunk.chunkInfo.IHDR.cInterlace) {
        case 0: {
            nRet = pResolution->pxHeight * (1 + (pResolution->pxWidth * nPixelSize));
        } break;
        case 1: {
            nRet = 0;
            for (uint8_t iLoop = 0; iLoop < PICTURE_PNG_ADAM7_SIZE; iLoop++)
            {
                GetAdam7Resolution(&pxResolution, pResolution, iLoop);
                nRet += pxResolution.pxHeight * (1 + (pxResolution.pxWidth * nPixelSize));
            }
        } break;

        default: { nRet = 0; } break;
    }

    return nRet;
}
static HANSIZE ReadPngChunk(const uint8_t* pData, HANSIZE nLen, PPICTUREPNGCHUNK pChunk)
{
    HANSIZE nRet = 0;
    BOOL bOk = TRUE;
    PICTUREPNGCHUNK pngChunk;
    HANSIZE nOffset = 0;
    uint32_t nDataLen;
    uint32_t cCrc;

    nDataLen = ReadPngData4ByteMSB(pData);
    if (nLen < ChunkDataLenToChunkLen(nDataLen)) { bOk = FALSE; }

    if (TRUE == bOk)
    {
        nOffset += 4;
        pngChunk.nLength = nDataLen;
        memcpy(pngChunk.pTypeCode, &pData[nOffset], 4);
        pngChunk.pData = &pData[nOffset + 4];
        cCrc = CRC32_STD(&pData[nOffset], nDataLen + 4);
        nOffset += 4 + nDataLen;
        pngChunk.cCrc = ReadPngData4ByteMSB(&pData[nOffset]);
        if (cCrc == pngChunk.cCrc) { pngChunk.bCrcOk = TRUE; }
        else { pngChunk.bCrcOk = FALSE; bOk = FALSE; }
    }

    if (TRUE == bOk)
    {
        *pChunk = pngChunk;
        nRet = ChunkDataLenToChunkLen(nDataLen);
    }
    return nRet;
}
static inline HANSIZE ChunkDataLenToChunkLen(uint32_t nChunkDataLen)
{
    return (nChunkDataLen + 12);
}
static PICTUREPNGCHUNKTYPE GetPngChunkType(HANPCSTR pType)
{
    PICTUREPNGCHUNKTYPE eRet = PICTURE_PNG_CHUNK_TYPE_CNT;

    for (PICTUREPNGCHUNKTYPE eType = 0; eType < PICTURE_PNG_CHUNK_TYPE_CNT; eType++)
    {
        if (!memcmp(sg_pPngChunkType[eType].pCode, pType, sizeof(HANCHAR) * 4))
        {
            eRet = eType;
        }
    }

    return eRet;
}
static void SetChunkMap(const uint8_t* pData, HANINT nChunkCnt, PPICTUREPNGWNDEXTRA pngInfo)
{
    HANSIZE nChunkPos = sizeof(sg_pPngHeader);
    uint32_t nChunkDataLen;
    HANCHAR pType[5];

    for (HANINT iLoop = 0; iLoop < nChunkCnt; iLoop++)
    {
        nChunkDataLen = ReadPngData4ByteMSB(&pData[nChunkPos]);

        memcpy(pngInfo->chunk.map.pChunkList[iLoop].pTypeCode, &pData[nChunkPos + 4], 4);
        HAN_snprintf(pType, ArrLen(pType), TEXT("%c%c%c%c"), pData[nChunkPos + 4], pData[nChunkPos + 5], pData[nChunkPos + 6], pData[nChunkPos + 7]);
        pngInfo->chunk.map.pChunkList[iLoop].eType = GetPngChunkType(pType);
        pngInfo->chunk.map.pChunkList[iLoop].nLength = nChunkDataLen;
        pngInfo->chunk.map.pChunkList[iLoop].pData = &pData[nChunkPos + 8];

        nChunkPos += ChunkDataLenToChunkLen(nChunkDataLen);
    }
}
static void MergeIDATData(PPICTUREPNGWNDEXTRA pngInfo)
{
    HANSIZE nChunkCnt = pngInfo->chunk.map.nCnt;
    PPICTUREPNGCHUNK pChunkList = pngInfo->chunk.map.pChunkList;
    uint8_t* pIDATData = pngInfo->pictureData.pIDATData;
    HANSIZE nOffset = 0;

    for (HANSIZE iLoop = 0; iLoop < nChunkCnt; iLoop++)
    {
        if (PICTURE_PNG_CHUNK_TYPE_IDAT == pChunkList[iLoop].eType)
        {
            memcpy(&pIDATData[nOffset], pChunkList[iLoop].pData, pChunkList[iLoop].nLength);
            nOffset += pChunkList[iLoop].nLength;
        }
    }
}
static void GetPngShowSize(PPICTUREPNGWNDEXTRA pngInfo, HANINT* pW, HANINT* pH)
{
    HANDOUBLE pxWidth = (HANDOUBLE)(pngInfo->chunk.chunkInfo.IHDR.pxResolution.pxWidth);
    HANDOUBLE pxHeight = (HANDOUBLE)(pngInfo->chunk.chunkInfo.IHDR.pxResolution.pxHeight);
    
    *pH = PICTURE_PNG_INFO_HEIGHT;
    *pW = (HANINT)((HANDOUBLE)(*pH) / pxHeight * pxWidth);
}
static void PicturePngPrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen)
{
    HANSIZE nLoopLen = PICTURE_HEX_PRINT_LEN;
    HANSIZE nOffset = 0;

    HAN_strcpy(pText, TEXT(""));
    if (nDataLen < PICTURE_HEX_PRINT_LEN) { nLoopLen = nDataLen; }
    for (HANSIZE iLoop = 0; iLoop < nLoopLen; iLoop++)
    {
        HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("%02X "), pData[iLoop]);
        nOffset += HAN_strlen(&pText[nOffset]);
    }
    if (PICTURE_HEX_PRINT_LEN < nDataLen) { HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("...")); }
}
static LRESULT ChunkListNotifyCallback(PPICTUREPNGWNDEXTRA pngInfo, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->code) {
        case NM_CLICK: {
            NMITEMACTIVATE* pItemAct = (NMITEMACTIVATE*)pNotify;
            if (-1 != pItemAct->iItem) { UpdateChunkInfoWindow(pngInfo, pItemAct->iItem); }
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void UpdateChunkInfoWindow(PPICTUREPNGWNDEXTRA pngInfo, HANINT nId)
{
    PICTUREPNGCHUNKTYPE eType = pngInfo->chunk.map.pChunkList[nId].eType;

    SendMessage(pngInfo->chunk.hInfo, WM_SETREDRAW, FALSE, 0);

    ListView_DeleteAllItems(pngInfo->chunk.hInfo);
    if ((0 <= eType) && (eType < PICTURE_PNG_CHUNK_TYPE_CNT))
    {
        sg_pPngChunkType[eType].UpdateChunkInfoWindow(&(pngInfo->chunk.map.pChunkList[nId]), &(pngInfo->chunk.chunkInfo), pngInfo->chunk.hInfo);
    }
    else
    {
        UpdateChunkInfoWindow_Default(pngInfo->chunk.map.pChunkList[nId].pTypeCode, pngInfo->chunk.hInfo);
    }

    SendMessage(pngInfo->chunk.hInfo, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(pngInfo->chunk.hInfo, NULL, TRUE);
}

static inline HANINT GetChunkListWindowWidth(void)
{
    HANINT nRet = 0;
    for (PICTUREPNGCHUNKLISTHEADER iLoop = 0; iLoop < PICTURE_PNG_CHUNK_LIST_HEADER_CNT; iLoop++)
    {
        nRet += sg_pPngChunkListHeader[iLoop].nWidth;
    }
    return nRet;
}
static inline HANINT GetChunkInfoWindowWidth(void)
{
    HANINT nRet = 0;
    for (PICTUREPNGCHUNKINFOHEADER iLoop = 0; iLoop < PICTURE_PNG_CHUNK_INFO_HEADER_CNT; iLoop++)
    {
        nRet += sg_pPngChunkInfoHeaderWidth[iLoop];
    }
    return nRet;
}
static inline uint32_t ReadPngData4ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}
static inline uint16_t ReadPngData2ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline void WritePngData4ByteMSB(uint8_t* pData, uint32_t nData)
{
    pData[0] = nData >> 24;
    pData[1] = nData >> 16;
    pData[2] = nData >> 8;
    pData[3] = nData;
}
static inline void WritePngData2ByteMSB(uint8_t* pData, uint32_t nData)
{
    pData[0] = nData >> 8;
    pData[1] = nData;
}
static inline PNGCOLOR ReadPngColor(const uint8_t* pData, uint8_t nColorSize)
{
    PNGCOLOR cRet = 0;
    
    switch (nColorSize) {
        case 1: { cRet = pData[0]; } break;
        case 2: { cRet = ReadPngData2ByteMSB(pData); } break;
        default: { cRet = 0; } break;
    }

    return cRet;
}
static inline void WritePngColor(uint8_t* pDest, PNGCOLOR cColor, uint8_t nColorSize)
{
    switch (nColorSize) {
        case 1: { pDest[0] = (uint8_t)cColor; } break;
        case 2: { WritePngData2ByteMSB(pDest, cColor); } break;
        default: { } break;
    }
}

static BOOL UpdateChunkInfo_Default(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    (void)pChunk;
    (void)pngInfo;
    return TRUE;
}
static BOOL UpdateChunkInfo_IHDR(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);

    pChunkInfo->IHDR.bValid = TRUE;
    pChunkInfo->IHDR.pxResolution.pxWidth = ReadPngData4ByteMSB(&pData[0]);
    pChunkInfo->IHDR.pxResolution.pxHeight = ReadPngData4ByteMSB(&pData[4]);
    pChunkInfo->IHDR.cBitDepth = pData[8];
    pChunkInfo->IHDR.cColorType = pData[9];
    pChunkInfo->IHDR.cCompression = pData[10];
    pChunkInfo->IHDR.cFilter = pData[11];
    pChunkInfo->IHDR.cInterlace = pData[12];

    pngInfo->pictureData.cColorType = pngInfo->chunk.chunkInfo.IHDR.cColorType;
    switch (pngInfo->pictureData.cColorType)
    {
        case HAN_PICTURE_PNG_COLOR_TYPE_GRAY_SCALE: { pngInfo->pictureData.pixelInfo.nColorCnt = 1; } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_TRUE_COLOR: { pngInfo->pictureData.pixelInfo.nColorCnt = 3; } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_INDEX_COLOR: { pngInfo->pictureData.pixelInfo.nColorCnt = 1; } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_GRAY_SCALE: { pngInfo->pictureData.pixelInfo.nColorCnt = 2; } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_TRUE_COLOR: { pngInfo->pictureData.pixelInfo.nColorCnt = 4; } break;
        default: { bRet = FALSE; } break;
    }
    switch (pngInfo->chunk.chunkInfo.IHDR.cBitDepth) {
        case 1: { pngInfo->pictureData.pixelInfo.nColorSize = 1; } break;
        case 2: { pngInfo->pictureData.pixelInfo.nColorSize = 1; } break;
        case 4: { pngInfo->pictureData.pixelInfo.nColorSize = 1; } break;
        case 8: { pngInfo->pictureData.pixelInfo.nColorSize = 1; } break;
        case 16: { pngInfo->pictureData.pixelInfo.nColorSize = 2; } break;
        default: { bRet = FALSE; } break;
    }
    if (TRUE == bRet)
    {
        pngInfo->pictureData.pixelInfo.nPixelSize = pngInfo->pictureData.pixelInfo.nColorCnt * pngInfo->pictureData.pixelInfo.nColorSize;
    }

    return bRet;
}
static BOOL UpdateChunkInfo_gAMA(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);

    pChunkInfo->gAMA.bValid = TRUE;
    pChunkInfo->gAMA.nValue = (HANDOUBLE)ReadPngData4ByteMSB(pChunk->pData) / (HANDOUBLE)100000;

    return TRUE;
}
static BOOL UpdateChunkInfo_pHYs(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);

    pChunkInfo->pHYs.bValid = TRUE;
    pChunkInfo->pHYs.pxResolutionH = ReadPngData4ByteMSB(&pData[0]);
    pChunkInfo->pHYs.pxResolutionV = ReadPngData4ByteMSB(&pData[4]);
    pChunkInfo->pHYs.cUnit = pData[8];

    return TRUE;
}
static BOOL UpdateChunkInfo_bKGD(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);
    uint8_t nIndex;

    pChunkInfo->bKGD.bValid = TRUE;
    switch (pChunkInfo->IHDR.cColorType) {
        case HAN_PICTURE_PNG_COLOR_TYPE_GRAY_SCALE: {
            pChunkInfo->bKGD.red = ReadPngData2ByteMSB(&(pChunk->pData[0]));
            pChunkInfo->bKGD.green = ReadPngData2ByteMSB(&(pChunk->pData[0]));
            pChunkInfo->bKGD.blue = ReadPngData2ByteMSB(&(pChunk->pData[0]));
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_TRUE_COLOR: {
            pChunkInfo->bKGD.red = ReadPngData2ByteMSB(&(pChunk->pData[0]));
            pChunkInfo->bKGD.green = ReadPngData2ByteMSB(&(pChunk->pData[2]));
            pChunkInfo->bKGD.blue = ReadPngData2ByteMSB(&(pChunk->pData[4]));
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_INDEX_COLOR: {
            nIndex = pChunk->pData[0];
            pChunkInfo->bKGD.red = pChunkInfo->PLTE.pData[nIndex * 3 + 0] * 65535 / 255;
            pChunkInfo->bKGD.green = pChunkInfo->PLTE.pData[nIndex * 3 + 1] * 65535 / 255;
            pChunkInfo->bKGD.blue = pChunkInfo->PLTE.pData[nIndex * 3 + 2] * 65535 / 255;
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_GRAY_SCALE: {
            pChunkInfo->bKGD.red = ReadPngData2ByteMSB(&(pChunk->pData[0]));
            pChunkInfo->bKGD.green = ReadPngData2ByteMSB(&(pChunk->pData[0]));
            pChunkInfo->bKGD.blue = ReadPngData2ByteMSB(&(pChunk->pData[0]));
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_TRUE_COLOR: {
            pChunkInfo->bKGD.red = ReadPngData2ByteMSB(&(pChunk->pData[0]));
            pChunkInfo->bKGD.green = ReadPngData2ByteMSB(&(pChunk->pData[2]));
            pChunkInfo->bKGD.blue = ReadPngData2ByteMSB(&(pChunk->pData[4]));
        } break;

        default: { pChunkInfo->bKGD.bValid = FALSE; } break;
    }

    return TRUE;
}
static BOOL UpdateChunkInfo_tIME(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);

    pChunkInfo->tIME.bValid = TRUE;
    pChunkInfo->tIME.nYear = ReadPngData2ByteMSB(pData);
    pChunkInfo->tIME.nMonth = pData[2];
    pChunkInfo->tIME.nDay = pData[3];
    pChunkInfo->tIME.nHour = pData[4];
    pChunkInfo->tIME.nMinute = pData[5];
    pChunkInfo->tIME.nSecond = pData[6];

    return TRUE;
}
static BOOL UpdateChunkInfo_iCCP(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);
    uint32_t nLen = pChunk->nLength;
    uint32_t nPrintLen = nLen;
    uint32_t nFileNameLen;

    if (PICTURE_HEX_PRINT_LEN < nPrintLen) { nPrintLen = PICTURE_HEX_PRINT_LEN; }

    nFileNameLen = (uint32_t)strlen((PCCH)pData);
    pChunkInfo->iCCP.bValid = TRUE;
    pChunkInfo->iCCP.pFileName = (PCHAR)(&pData[0]);
    pChunkInfo->iCCP.cCompression = pData[nFileNameLen];
    pChunkInfo->iCCP.pData = &pData[nFileNameLen + 1];
    pChunkInfo->iCCP.nDataLen = nLen - nFileNameLen - 1;

    return TRUE;
}
static BOOL UpdateChunkInfo_cHRM(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);

    pChunkInfo->cHRM.bValid = TRUE;
    pChunkInfo->cHRM.whiteX = ReadPngData4ByteMSB(&pData[0]);
    pChunkInfo->cHRM.whiteY = ReadPngData4ByteMSB(&pData[4]);
    pChunkInfo->cHRM.redX = ReadPngData4ByteMSB(&pData[8]);
    pChunkInfo->cHRM.redY = ReadPngData4ByteMSB(&pData[12]);
    pChunkInfo->cHRM.greenX = ReadPngData4ByteMSB(&pData[16]);
    pChunkInfo->cHRM.greenY = ReadPngData4ByteMSB(&pData[20]);
    pChunkInfo->cHRM.blueX = ReadPngData4ByteMSB(&pData[24]);
    pChunkInfo->cHRM.blueY = ReadPngData4ByteMSB(&pData[28]);

    return TRUE;
}
static BOOL UpdateChunkInfo_PLTE(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);
    uint32_t nLen = pChunk->nLength;
    uint32_t nPrintLen = nLen - (nLen % 3);

    if ((PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX * 3) < nPrintLen) { nPrintLen = PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX * 3; }

    pChunkInfo->PLTE.bValid = TRUE;
    pChunkInfo->PLTE.pData = pData;
    pChunkInfo->PLTE.nDataLen = nLen;

    return TRUE;
}
static BOOL UpdateChunkInfo_tRNS(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);
    uint32_t nLen = pChunk->nLength;
    uint32_t nPrintLen = nLen;

    if (PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX < nPrintLen) { nPrintLen = PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX; }

    pChunkInfo->tRNS.bValid = TRUE;
    pChunkInfo->tRNS.pData = pData;
    pChunkInfo->tRNS.nDataLen = nLen;

    return TRUE;
}
static BOOL UpdateChunkInfo_IDAT(PCPICTUREPNGCHUNK pChunk, PPICTUREPNGWNDEXTRA pngInfo)
{
    const uint8_t* pData = pChunk->pData;
    PPICTUREPNGCHUNKINFO pChunkInfo = &(pngInfo->chunk.chunkInfo);
    uint32_t nLen = pChunk->nLength;
    uint32_t nPrintLen = nLen;

    if (PICTURE_HEX_PRINT_LEN < nPrintLen) { nPrintLen = PICTURE_HEX_PRINT_LEN; }

    pChunkInfo->IDAT.bValid = TRUE;
    pChunkInfo->IDAT.pData = pData;
    pChunkInfo->IDAT.nDataLen = nLen;

    pngInfo->pictureData.nIDATDataLen += nLen;

    return TRUE;
}

static void UpdateChunkInfoWindow_Default(const PCHAR pCode, HWND hListView)
{
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE];
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = pText;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%c%c%c%c"), pCode[0], pCode[1], pCode[2], pCode[3]);
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = TEXT("不支持该数据块");
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);
}
static void UpdateChunkInfoWindow_IHDR(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    const uint8_t* pData = pChunkList->pData;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_IHDR].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_IHDR_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_IHDR iLoop = 0; iLoop < PICTURE_PNG_IHDR_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_IHDR_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 分辨率 */
    lvItem.iItem = PICTURE_PNG_IHDR_CHUNK_FIELD_RESOLUTION;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u×%u"), ReadPngData4ByteMSB(&pData[0]), ReadPngData4ByteMSB(&pData[4]));
    ListView_SetItem(hListView, &lvItem);
    /* 位深度 */
    lvItem.iItem = PICTURE_PNG_IHDR_CHUNK_FIELD_BIT_DEPTH;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u（%u位深）"), pData[8], (uint32_t)((uint32_t)1 << (uint32_t)pData[8]));
    ListView_SetItem(hListView, &lvItem);
    /* 颜色类型 */
    lvItem.iItem = PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pData[9], GetPng_IHDR_ColorTypeName(pData[9]));
    ListView_SetItem(hListView, &lvItem);
    /* 压缩方法 */
    lvItem.iItem = PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pData[10], GetPng_IHDR_CompressionName(pData[10]));
    ListView_SetItem(hListView, &lvItem);
    /* 滤波方法 */
    lvItem.iItem = PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pData[11], GetPng_IHDR_FilterName(pData[11]));
    ListView_SetItem(hListView, &lvItem);
    /* 隔行扫描 */
    lvItem.iItem = PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pData[12], GetPng_IHDR_InterlaceName(pData[12]));
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_gAMA(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_gAMA].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_gAMA_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_gAMA iLoop = 0; iLoop < PICTURE_PNG_gAMA_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_gAMA_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* Gamma值 */
    lvItem.iItem = PICTURE_PNG_gAMA_CHUNK_FIELD_VALUE;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)ReadPngData4ByteMSB(pChunkList->pData) / (HANDOUBLE)100000);
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_pHYs(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    HANDOUBLE phyWidth;
    HANDOUBLE phyHeight;
    PICTURERESOLUTION pxResolution;
    uint8_t cUnit = pChunkList->pData[8];
    HANPCSTR pUnit = GetPng_pHYs_UnitName(cUnit);
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pResolutionUnit[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_pHYs].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_pHYs_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_pHYs iLoop = 0; iLoop < PICTURE_PNG_pHYs_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_pHYs_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 物理单位 */
    lvItem.iItem = PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%s"), pUnit);
    if (1 == cUnit) { HAN_snprintf(pResolutionUnit, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT(" 像素/%s"), pUnit); }
    ListView_SetItem(hListView, &lvItem);
    /* 水平分辨率 */
    lvItem.iItem = PICTURE_PNG_pHYs_CHUNK_FIELD_RESOLUTION_H;
    pxResolution.pxWidth = ReadPngData4ByteMSB(&(pChunkList->pData[0]));
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u%s"), pxResolution.pxWidth, pResolutionUnit);
    ListView_SetItem(hListView, &lvItem);
    /* 垂直分辨率 */
    lvItem.iItem = PICTURE_PNG_pHYs_CHUNK_FIELD_RESOLUTION_V;
    pxResolution.pxHeight = ReadPngData4ByteMSB(&(pChunkList->pData[4]));
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u%s"), pxResolution.pxHeight, pResolutionUnit);
    ListView_SetItem(hListView, &lvItem);
    /* 物理尺寸 */
    if (1 == cUnit)
    {
        phyWidth = (HANDOUBLE)(pChunkInfo->IHDR.pxResolution.pxWidth) / (HANDOUBLE)(pxResolution.pxWidth);
        phyHeight = (HANDOUBLE)(pChunkInfo->IHDR.pxResolution.pxHeight) / (HANDOUBLE)(pxResolution.pxHeight);
        lvItem.iItem = PICTURE_PNG_pHYs_CHUNK_FIELD_PHY_SIZE;
        HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%.2lf×%.2lf"), phyWidth, phyHeight);
        ListView_SetItem(hListView, &lvItem);
    }
}
static void UpdateChunkInfoWindow_bKGD(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    BOOL bOk = TRUE;
    const uint8_t* pData = pChunkList->pData;
    uint8_t nIndex;
    uint16_t pRGB[3];
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_bKGD].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_bKGD_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_bKGD iLoop = 0; iLoop < PICTURE_PNG_bKGD_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_bKGD_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 背景色 */
    lvItem.iItem = PICTURE_PNG_bKGD_CHUNK_FIELD_BACKGROUND;
    switch (pChunkInfo->IHDR.cColorType) {
        case HAN_PICTURE_PNG_COLOR_TYPE_GRAY_SCALE: {
            pRGB[0] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
            pRGB[1] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
            pRGB[2] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_TRUE_COLOR: {
            pRGB[0] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
            pRGB[1] = ReadPngData2ByteMSB(&(pData[2])) * 255 / 65535;
            pRGB[2] = ReadPngData2ByteMSB(&(pData[4])) * 255 / 65535;
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_INDEX_COLOR: {
            nIndex = pData[0];
            pRGB[0] = pChunkInfo->PLTE.pData[nIndex * 3 + 0];
            pRGB[1] = pChunkInfo->PLTE.pData[nIndex * 3 + 1];
            pRGB[2] = pChunkInfo->PLTE.pData[nIndex * 3 + 2];
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_GRAY_SCALE: {
            pRGB[0] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
            pRGB[1] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
            pRGB[2] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_TRUE_COLOR: {
            pRGB[0] = ReadPngData2ByteMSB(&(pData[0])) * 255 / 65535;
            pRGB[1] = ReadPngData2ByteMSB(&(pData[2])) * 255 / 65535;
            pRGB[2] = ReadPngData2ByteMSB(&(pData[4])) * 255 / 65535;
        } break;

        default: { bOk = FALSE; } break;
    }
    if (TRUE == bOk) { HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("#%02X%02X%02X"), pRGB[0], pRGB[1], pRGB[2]); }
    else { HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("颜色类型错误")); }
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_tIME(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    const uint8_t* pData = pChunkList->pData;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_tIME].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_tIME_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_tIME iLoop = 0; iLoop < PICTURE_PNG_tIME_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_tIME_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }
    
    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* UTC */
    lvItem.iItem = PICTURE_PNG_tIME_CHUNK_FIELD_UTC;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u/%u/%u %u:%u:%u"),
        ReadPngData2ByteMSB(pData), pData[2], pData[3], pData[4], pData[5], pData[6]
    );
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_iCCP(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    size_t nFileNameLen;
    const uint8_t* pData = pChunkList->pData;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_iCCP].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_iCCP_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_iCCP iLoop = 0; iLoop < PICTURE_PNG_iCCP_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_iCCP_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }
    
    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 文件名 */
    lvItem.iItem = PICTURE_PNG_iCCP_CHUNK_FIELD_FILE_NAME;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pData);
    ListView_SetItem(hListView, &lvItem);
    /* 压缩方法 */
    lvItem.iItem = PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION;
    nFileNameLen = strlen((const CHAR*)pData);
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pData[nFileNameLen], GetPng_iCCP_CompressionName(pData[nFileNameLen]));
    ListView_SetItem(hListView, &lvItem);
    /* 数据 */
    lvItem.iItem = PICTURE_PNG_iCCP_CHUNK_FIELD_DATA;
    PicturePngPrintHexData(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, &pData[nFileNameLen + 1], pChunkList->nLength - nFileNameLen - 1);
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_cHRM(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    HANDOUBLE colorX;
    HANDOUBLE colorY;
    const uint8_t* pData = pChunkList->pData;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_cHRM].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_cHRM_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_cHRM iLoop = 0; iLoop < PICTURE_PNG_cHRM_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_cHRM_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }
    
    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 白色 */
    colorX = (DOUBLE)ReadPngData4ByteMSB(&pData[0]) / (DOUBLE)100000;
    colorY = (DOUBLE)ReadPngData4ByteMSB(&pData[4]) / (DOUBLE)100000;
    lvItem.iItem = PICTURE_PNG_cHRM_CHUNK_FIELD_WHITE_POSITION;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("(%.6lf,%.6lf)"), colorX, colorY);
    ListView_SetItem(hListView, &lvItem);
    /* 红色 */
    colorX = (DOUBLE)ReadPngData4ByteMSB(&pData[8]) / (DOUBLE)100000;
    colorY = (DOUBLE)ReadPngData4ByteMSB(&pData[12]) / (DOUBLE)100000;
    lvItem.iItem = PICTURE_PNG_cHRM_CHUNK_FIELD_RED_POSITION;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("(%.6lf,%.6lf)"), colorX, colorY);
    ListView_SetItem(hListView, &lvItem);
    /* 绿色 */
    colorX = (DOUBLE)ReadPngData4ByteMSB(&pData[16]) / (DOUBLE)100000;
    colorY = (DOUBLE)ReadPngData4ByteMSB(&pData[20]) / (DOUBLE)100000;
    lvItem.iItem = PICTURE_PNG_cHRM_CHUNK_FIELD_GREEN_POSITION;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("(%.6lf,%.6lf)"), colorX, colorY);
    ListView_SetItem(hListView, &lvItem);
    /* 蓝色 */
    colorX = (DOUBLE)ReadPngData4ByteMSB(&pData[24]) / (DOUBLE)100000;
    colorY = (DOUBLE)ReadPngData4ByteMSB(&pData[28]) / (DOUBLE)100000;
    lvItem.iItem = PICTURE_PNG_cHRM_CHUNK_FIELD_BLUE_POSITION;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("(%.6lf,%.6lf)"), colorX, colorY);
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_PLTE(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    uint32_t nLen = (pChunkList->nLength - (pChunkList->nLength % 3)) / 3;
    const uint8_t* pRGB;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_PLTE].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_PLTE_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.pszText = pText;
    if (PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX < nLen) { nLen = PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX; }
    for (uint32_t iLoop = 0; iLoop < nLen; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
        HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("颜色%u"), iLoop);
        ListView_InsertItem(hListView, &lvItem);
        
        pRGB = &pChunkList->pData[iLoop * 3];
        lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
        HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("#%02X%02X%02X"), pRGB[0], pRGB[1], pRGB[2]);
        ListView_SetItem(hListView, &lvItem);
    }
}
static void UpdateChunkInfoWindow_tRNS(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    uint32_t nLen = pChunkList->nLength;
    const uint8_t* pData = pChunkList->pData;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_tRNS].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_tRNS_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.pszText = pText;
    if (PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX < nLen) { nLen = PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX; }
    switch (pChunkInfo->IHDR.cColorType) {
        case HAN_PICTURE_PNG_COLOR_TYPE_GRAY_SCALE: { // 0（灰度图像）
            lvItem.iItem = 0;
            lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
            HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("透明灰度"));
            ListView_InsertItem(hListView, &lvItem);
            
            lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
            HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u"), ReadPngData2ByteMSB(&pData[0]));
            ListView_SetItem(hListView, &lvItem);
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_TRUE_COLOR: { // 2（真彩色图像）
            lvItem.iItem = 0;
            lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
            HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("透明色"));
            ListView_InsertItem(hListView, &lvItem);
            
            lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
            HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u,%u,%u"),
                ReadPngData2ByteMSB(&pData[0]),
                ReadPngData2ByteMSB(&pData[2]),
                ReadPngData2ByteMSB(&pData[4])
            );
            ListView_SetItem(hListView, &lvItem);
        } break;
        case HAN_PICTURE_PNG_COLOR_TYPE_INDEX_COLOR: { // 3（索引颜色图像）
            for (uint32_t iLoop = 0; iLoop < nLen; iLoop++)
            {
                lvItem.iItem = iLoop;
                lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
                HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("透明度%u"), iLoop);
                ListView_InsertItem(hListView, &lvItem);
                
                lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
                HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u"), pData[iLoop]);
                ListView_SetItem(hListView, &lvItem);
            }
        } break;

        default: {
            lvItem.iItem = 0;
            lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
            HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("未知透明数据"));
            ListView_InsertItem(hListView, &lvItem);
            
            lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
            PicturePngPrintHexData(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, pData, pChunkInfo->tRNS.nDataLen);
            ListView_SetItem(hListView, &lvItem);
        } break;
    }
}
static void UpdateChunkInfoWindow_IDAT(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_IDAT].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_IDAT_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    for (PICTUREPNGCHUNKFIELD_IDAT iLoop = 0; iLoop < PICTURE_PNG_IDAT_CHUNK_FIELD_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = GetPng_IDAT_FieldName(iLoop);
        ListView_InsertItem(hListView, &lvItem);
    }
    
    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    lvItem.pszText = pText;
    /* 长度 */
    lvItem.iItem = PICTURE_PNG_IDAT_CHUNK_FIELD_LEN;
    HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, TEXT("%u"), pChunkList->nLength);
    ListView_SetItem(hListView, &lvItem);
    /* 数据 */
    lvItem.iItem = PICTURE_PNG_IDAT_CHUNK_FIELD_DATA;
    PicturePngPrintHexData(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, pChunkList->pData, pChunkList->nLength);
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_tEXT(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkInfo;
    HANSIZE nKeyLen;
    HANCHAR pText[HAN_PICTURE_PNG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_tEXT].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = GetPng_tEXT_Name();
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);

    lvItem.iItem = 0;
    lvItem.pszText = pText;

    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_FIELD;
    nKeyLen = HAN_snprintf(pText, HAN_PICTURE_PNG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pChunkList->pData);
    ListView_InsertItem(hListView, &lvItem);
    
    lvItem.iSubItem = PICTURE_PNG_CHUNK_INFO_HEADER_VALUE;
    HAN_snprintf(pText, pChunkList->nLength - nKeyLen - 1, HANPSTR_PRINT_PCHAR_FORMAT, &(pChunkList->pData[nKeyLen + 1]));
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateChunkInfoWindow_IEND(PPICTUREPNGCHUNK pChunkList, PCPICTUREPNGCHUNKINFO pChunkInfo, HWND hListView)
{
    (void)pChunkList;
    (void)pChunkInfo;
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    lvTitle.pszText = sg_pPngChunkType[PICTURE_PNG_CHUNK_TYPE_IEND].pCode;
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = TEXT("图像结束");
    ListView_SetColumn(hListView, PICTURE_PNG_CHUNK_INFO_HEADER_VALUE, &lvTitle);
}

static BOOL DecodeIDAT(PPICTUREPNGWNDEXTRA pngInfo)
{
    BOOL bRet = TRUE;
    int nUncompressRet = 0;
    Bytef* pUncompressedData = pngInfo->pictureData.pColorData;
    uLongf nUncompressedLen = (uLongf)pngInfo->pictureData.nColorDataLen;
    const Bytef* pCompressedData = pngInfo->pictureData.pIDATData;
    uLong nCompressedLen = (uLongf)pngInfo->pictureData.nIDATDataLen;
    uint8_t nPixelSize = pngInfo->pictureData.pixelInfo.nPixelSize;
    PCPICTURERESOLUTION pResolution = &(pngInfo->chunk.chunkInfo.IHDR.pxResolution);
    PICTURERESOLUTION pxResolution;
    uint32_t nOffset;

    nUncompressRet = uncompress(pUncompressedData, &nUncompressedLen, pCompressedData, nCompressedLen);
    if (0 != nUncompressRet) { bRet = FALSE; }
    if (TRUE == bRet)
    {
        switch (pngInfo->chunk.chunkInfo.IHDR.cInterlace) {
            case 0: {
                PngFilterDecode(pUncompressedData, pResolution, &(pngInfo->pictureData.pixelInfo));
            } break;
            case 1: {
                nOffset = 0;
                for (uint8_t iLoop = 0; iLoop < PICTURE_PNG_ADAM7_SIZE; iLoop++)
                {
                    GetAdam7Resolution(&pxResolution, pResolution, iLoop);
                    PngFilterDecode(&pUncompressedData[nOffset], &pxResolution, &(pngInfo->pictureData.pixelInfo));
                    nOffset += pxResolution.pxHeight * (1 + (pxResolution.pxWidth * nPixelSize));
                }
            } break;
            
            default: { bRet = FALSE; } break;
        }
    }
    if (TRUE == bRet) { PngArrangePixel(pngInfo); }

    return bRet;
}
static BOOL PngFilterDecode(uint8_t* pData, PCPICTURERESOLUTION pResolution, PPICTUREPIXELINFO pPixelInfo)
{
    BOOL bRet = TRUE;
    uint8_t* pPreRow;
    uint8_t* pCurRow;
    uint8_t nColorSize = pPixelInfo->nColorSize;
    uint8_t nPixelSize = pPixelInfo->nPixelSize;
    uint32_t pxWidth = pResolution->pxWidth;
    uint32_t pxHeight = pResolution->pxHeight;
    uint32_t nRowSize = pxWidth * nPixelSize;
    uint32_t nPictureSize = nRowSize * pxHeight;
    uint32_t idX;
    PNGCOLOR pxA;
    PNGCOLOR pxB;
    PNGCOLOR pxC;

    pPreRow = NULL;
    for (uint32_t idRow = 0; idRow < nPictureSize; idRow += 1 + nRowSize)
    {
        pCurRow = &pData[idRow + 1];
        switch (pData[idRow]) {
            case 0: { /* 无滤波，什么都不做 */ } break;
            case 1: { /* Sub 滤波，x = x + a */
                for (uint32_t idCol = 0; idCol < nRowSize; idCol += nPixelSize)
                {
                    for (uint32_t iLoop = 0; iLoop < nPixelSize; iLoop++)
                    {
                        pxA = 0;
                        idX = idCol + iLoop;
                        if (nPixelSize <= idCol) { pxA = pCurRow[idX - nPixelSize]; }
                        pCurRow[idX] += pxA;
                    }
                }
            } break;
            case 2: { /* Up 滤波，x = x + b */
                for (uint32_t idCol = 0; idCol < nRowSize; idCol += nPixelSize)
                {
                    for (uint32_t iLoop = 0; iLoop < nPixelSize; iLoop += nColorSize)
                    {
                        pxB = 0;
                        idX = idCol + iLoop;
                        if (NULL != pPreRow) { pxB = pPreRow[idX]; }
                        pCurRow[idX] += pxB;
                    }
                }
            } break;
            case 3: { /* Avg 滤波，x = x + ((a + b) / 2) */
                for (uint32_t idCol = 0; idCol < nRowSize; idCol += nPixelSize)
                {
                    for (uint32_t iLoop = 0; iLoop < nPixelSize; iLoop += nColorSize)
                    {
                        pxA = 0;
                        pxB = 0;
                        idX = idCol + iLoop;
                        if (nPixelSize <= idCol) { pxA = pCurRow[idX - nPixelSize]; }
                        if (NULL != pPreRow) { pxB = pPreRow[idX]; }
                        pCurRow[idX] += (pxA + pxB) / 2;
                    }
                }
            } break;
            case 4: { /* Paeth 滤波，x = x + Paeth(a, b, c) */
                for (uint32_t idCol = 0; idCol < nRowSize; idCol += nPixelSize)
                {
                    for (uint32_t iLoop = 0; iLoop < nPixelSize; iLoop += nColorSize)
                    {
                        pxA = 0;
                        pxB = 0;
                        pxC = 0;
                        idX = idCol + iLoop;
                        if (nPixelSize <= idCol) { pxA = pCurRow[idX - nPixelSize]; }
                        if (NULL != pPreRow) { pxB = pPreRow[idX]; }
                        if ((nPixelSize <= idCol) && (NULL != pPreRow)) { pxC = pPreRow[idX - nPixelSize]; }
                        pCurRow[idX] += PngFilterPaeth(pxA, pxB, pxC);
                    }
                }
            } break;

            default: { bRet = FALSE; } break;
        }
        pPreRow = pCurRow;
    }

    return bRet;
}
static void GetAdam7Resolution(PPICTURERESOLUTION pDestResolution, PCPICTURERESOLUTION pSrcResolution, uint8_t nLevel)
{
    pDestResolution->pxWidth = (pSrcResolution->pxWidth - sg_pxAdam7StartH[nLevel]) / sg_pxAdam7DistanceH[nLevel];
    pDestResolution->pxHeight = (pSrcResolution->pxHeight - sg_pxAdam7StartV[nLevel]) / sg_pxAdam7DistanceV[nLevel];
    if (0 != ((pSrcResolution->pxWidth - sg_pxAdam7StartH[nLevel]) % sg_pxAdam7DistanceH[nLevel])) { pDestResolution->pxWidth++; }
    if (0 != ((pSrcResolution->pxHeight - sg_pxAdam7StartV[nLevel]) % sg_pxAdam7DistanceV[nLevel])) { pDestResolution->pxHeight++; }
}
static inline PNGCOLOR PngFilterPaeth(PNGCOLOR pxA, PNGCOLOR pxB, PNGCOLOR pxC)
{
    PNGCOLOR nRet;
    INT nA = (INT)pxA;
    INT nB = (INT)pxB;
    INT nC = (INT)pxC;
    INT nP = nA + nB - nC;
    UINT uA = abs(nP - nA);
    UINT uB = abs(nP - nB);
    UINT uC = abs(nP - nC);

    if ((uA <= uB) && (uA <= uC)) { nRet = pxA; }
    else if (uB <= uC) { nRet = pxB; }
    else { nRet = pxC; }

    return nRet;
}
static void PngArrangePixel(PPICTUREPNGWNDEXTRA pngInfo)
{
    switch (pngInfo->chunk.chunkInfo.IHDR.cInterlace) {
        case 0: { PngArrangePixelNoInterlace(pngInfo); } break;
        case 1: { PngArrangePixelInterlace(pngInfo); } break;
        
        default: { } break;
    }
}
static void PngArrangePixelNoInterlace(PPICTUREPNGWNDEXTRA pngInfo)
{
    HANPPICTURE pPicture = pngInfo->pictureData.pPictureInfo->pPicture[0];
    uint8_t nPixelSize = pngInfo->pictureData.pixelInfo.nPixelSize;
    PICTURERESOLUTION pxResolution = pngInfo->chunk.chunkInfo.IHDR.pxResolution;
    uint8_t* pData = pngInfo->pictureData.pColorData;
    uint8_t* pRow;
    uint32_t nRowSize = 1 + (pxResolution.pxWidth * nPixelSize);
    uint32_t idCol;

    for (uint32_t nRow = 0; nRow < pxResolution.pxHeight; nRow++)
    {
        pRow = &pData[(nRow * nRowSize) + 1];
        for (uint32_t nCol = 0; nCol < pxResolution.pxWidth; nCol++)
        {
            idCol = nCol * nPixelSize;
            SetPictureRGBA(pngInfo, &(pPicture->pPictureMap[nRow][nCol]), &pRow[idCol]);
        }
    }
}
static void PngArrangePixelInterlace(PPICTUREPNGWNDEXTRA pngInfo)
{
    HANPPICTURE pPicture = pngInfo->pictureData.pPictureInfo->pPicture[0];
    uint8_t nPixelSize = pngInfo->pictureData.pixelInfo.nPixelSize;
    PICTURERESOLUTION pxResolution = pngInfo->chunk.chunkInfo.IHDR.pxResolution;
    PICTURERESOLUTION pxSubResolution = pngInfo->chunk.chunkInfo.IHDR.pxResolution;
    uint8_t* pData = pngInfo->pictureData.pColorData;
    uint8_t* pRow;
    uint32_t nSubRowSize;
    uint32_t nRow;
    uint32_t nCol;

    uint32_t idCol;

    pRow = &pData[1];
    for (uint8_t iLoop = 0; iLoop < PICTURE_PNG_ADAM7_SIZE; iLoop++)
    {
        GetAdam7Resolution(&pxSubResolution, &pxResolution, iLoop);
        nSubRowSize = 1 + (pxSubResolution.pxWidth * nPixelSize);
        nRow = sg_pxAdam7StartV[iLoop];
        for (uint32_t nSubRow = 0; nSubRow < pxSubResolution.pxHeight; nSubRow++)
        {
            nCol = sg_pxAdam7StartH[iLoop];
            for (uint32_t nSubCol = 0; nSubCol < pxSubResolution.pxWidth; nSubCol++)
            {
                idCol = nSubCol * nPixelSize;
                SetPictureRGBA(pngInfo, &(pPicture->pPictureMap[nRow][nCol]), &pRow[idCol]);
                nCol += sg_pxAdam7DistanceH[iLoop];
            }
            nRow += sg_pxAdam7DistanceV[iLoop];
            pRow = &pRow[nSubRowSize];
        }
    }
}
static void SetPictureRGBA(PPICTUREPNGWNDEXTRA pngInfo, PPICTURERGBA pRGBA, uint8_t* pData)
{
    BOOL bOk = TRUE;
    PNGCOLOR cMaxColor;
    uint8_t nColorSize = pngInfo->pictureData.pixelInfo.nColorSize;

    switch (pngInfo->pictureData.pixelInfo.nColorSize) {
        case 1: { cMaxColor = 0xFF; } break;
        case 2: { cMaxColor = 0xFFFF; } break;
        default: { bOk = FALSE; } break;
    }
    if (TRUE == bOk)
    {
        switch (pngInfo->pictureData.cColorType) {
            case HAN_PICTURE_PNG_COLOR_TYPE_GRAY_SCALE: {
                pRGBA->r = ReadPngColor(&pData[0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->g = ReadPngColor(&pData[0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->b = ReadPngColor(&pData[0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->a = 0xFF;
            } break;
            case HAN_PICTURE_PNG_COLOR_TYPE_TRUE_COLOR: {
                pRGBA->r = ReadPngColor(&pData[nColorSize * 0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->g = ReadPngColor(&pData[nColorSize * 1], nColorSize) * 0xFF / cMaxColor;
                pRGBA->b = ReadPngColor(&pData[nColorSize * 2], nColorSize) * 0xFF / cMaxColor;
                pRGBA->a = 0xFF;
            } break;
            case HAN_PICTURE_PNG_COLOR_TYPE_INDEX_COLOR: {
                const uint8_t* pPalette = pngInfo->chunk.chunkInfo.PLTE.pData;
                const uint8_t* pAlpha = NULL;
                if (TRUE == pngInfo->chunk.chunkInfo.tRNS.bValid) { pAlpha = pngInfo->chunk.chunkInfo.tRNS.pData; }
                
                pRGBA->r = ReadPngColor(&pPalette[(pData[0] * 3) + 0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->g = ReadPngColor(&pPalette[(pData[0] * 3) + 1], nColorSize) * 0xFF / cMaxColor;
                pRGBA->b = ReadPngColor(&pPalette[(pData[0] * 3) + 2], nColorSize) * 0xFF / cMaxColor;
                if (NULL != pAlpha) { pRGBA->a = pAlpha[pData[0]] * 0xFF / cMaxColor; }
                else { pRGBA->a = 0xFF; }
            } break;
            case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_GRAY_SCALE: {
                pRGBA->r = ReadPngColor(&pData[0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->g = ReadPngColor(&pData[0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->b = ReadPngColor(&pData[0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->a = ReadPngColor(&pData[nColorSize * 1], nColorSize) * 0xFF / cMaxColor;
            } break;
            case HAN_PICTURE_PNG_COLOR_TYPE_ALPHA_TRUE_COLOR: {
                pRGBA->r = ReadPngColor(&pData[nColorSize * 0], nColorSize) * 0xFF / cMaxColor;
                pRGBA->g = ReadPngColor(&pData[nColorSize * 1], nColorSize) * 0xFF / cMaxColor;
                pRGBA->b = ReadPngColor(&pData[nColorSize * 2], nColorSize) * 0xFF / cMaxColor;
                pRGBA->a = ReadPngColor(&pData[nColorSize * 3], nColorSize) * 0xFF / cMaxColor;
            } break;

            default: { } break;
        }
    }
}

static void SaveOriPictureInfo(PPICTUREPNGWNDEXTRA pngInfo, HANDLE hFile)
{
    (void)pngInfo;
    (void)hFile;
    const uint8_t* pData = pngInfo->paramPicture.pData;
    HANSIZE nLen = pngInfo->paramPicture.nLen;
    HANSIZE nOffset;
    HANSIZE nChunkLen;
    PICTUREPNGCHUNK pngChunk;
    PICTUREPNGCHUNKTYPE chunkType;
    HANCHAR pTypeCode[5];

    WriteFile(hFile, sg_pPngHeader, sizeof(sg_pPngHeader), NULL, NULL);
    nOffset = sizeof(sg_pPngHeader);
    while (nOffset < nLen)
    {
        nChunkLen = ReadPngChunk(&pData[nOffset], nLen - nOffset, &pngChunk);
        HAN_snprintf(pTypeCode, ArrLen(pTypeCode), TEXT("%c%c%c%c"), pngChunk.pTypeCode[0], pngChunk.pTypeCode[1], pngChunk.pTypeCode[2], pngChunk.pTypeCode[3]);
        chunkType = GetPngChunkType(pTypeCode);
        if ((PICTURE_PNG_CHUNK_TYPE_IDAT != chunkType) && (PICTURE_PNG_CHUNK_TYPE_IEND != chunkType))
        {
            WriteFile(hFile, &pData[nOffset], (DWORD)nChunkLen, NULL, NULL);
        }
        nOffset += nChunkLen;
    }
}
static BOOL SavePngChunk_IHDR(HANPPICTURE pPicture, HANDLE hFile)
{
    HANSIZE nOffset;
    HANSIZE nCrcStartOffset;
    uint32_t cCrc;
    uint8_t pData[25];
    
    nOffset = 0;
    WritePngData4ByteMSB(&pData[nOffset], 13); nOffset += 4;
    nCrcStartOffset = nOffset;
    memcpy(&pData[nOffset], "IHDR", 4); nOffset += 4;

    WritePngData4ByteMSB(&pData[nOffset], pPicture->pxResolution.pxWidth); nOffset += 4;
    WritePngData4ByteMSB(&pData[nOffset], pPicture->pxResolution.pxHeight); nOffset += 4;
    pData[nOffset] = 8; nOffset += 1;
    pData[nOffset] = 6; nOffset += 1;
    pData[nOffset] = 0; nOffset += 1;
    pData[nOffset] = 0; nOffset += 1;
    pData[nOffset] = 0; nOffset += 1;

    cCrc = CRC32_STD(&pData[nCrcStartOffset], 17);
    WritePngData4ByteMSB(&pData[nOffset], cCrc);

    return WriteFile(hFile, pData, sizeof(pData), NULL, NULL);
}
static BOOL SavePngChunk_IDAT(HANPPICTURE pPicture, HANDLE hHeap, HANDLE hFile)
{
    BOOL bRet = TRUE;
    PPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    uint8_t* pColor;
    uint8_t* pCompress = NULL;
    uint8_t* pTempCompress;
    HANSIZE nColorSize;
    HANSIZE nCompressSize;
    uLongf nCompressedSize;
    uint32_t cCrc;
    uint8_t pBytes[4];
    HANSIZE nWriteSize;
    HANSIZE nOffset;
    
    nColorSize = (1 + (pResolution->pxWidth * 4)) * pResolution->pxHeight;
    nCompressSize = nColorSize;
    pColor = (uint8_t*)HANWinHeapAlloc(hHeap, NULL, nColorSize);
    if (NULL != pColor)
    {
        InitIDATColorData(pColor, pPicture);

        while (TRUE == bRet)
        {
            /* 在压缩数据前面额外添加 4 个字节用来存放 IDAT 数据段名称，校验的时候就不用再找一个缓存整理数据了 */
            pTempCompress = (uint8_t*)HANWinHeapAlloc(hHeap, pCompress, nCompressSize + 4);
            if (NULL != pTempCompress)
            {
                pCompress = pTempCompress;
                nCompressedSize = (uLongf)nCompressSize;
                if (Z_OK == compress(&pCompress[4], &nCompressedSize, pColor, (uLong)nColorSize))
                {
                    break;
                }
                nCompressSize *= 2;
            }
            else { bRet = FALSE; }
        }
    }
    else { bRet = FALSE; }

    if (TRUE == bRet)
    {
        nOffset = 0;
        do {
            if ((nCompressedSize - nOffset) < PICTURE_PNG_IDAT_MAX_SIZE) { nWriteSize = nCompressedSize - nOffset; }
            else { nWriteSize = PICTURE_PNG_IDAT_MAX_SIZE; }

            memcpy(&pCompress[nOffset], "IDAT", 4);
            cCrc = CRC32_STD(&pCompress[nOffset], nWriteSize + 4);

            WritePngData4ByteMSB(pBytes, (uint32_t)nWriteSize);
            bRet = WriteFile(hFile, pBytes, 4, NULL, NULL);

            if (TRUE == bRet)
            {
                bRet = WriteFile(hFile, &pCompress[nOffset], (DWORD)(nWriteSize + 4), NULL, NULL);
            }

            if (TRUE == bRet)
            {
                WritePngData4ByteMSB(pBytes, cCrc);
                bRet = WriteFile(hFile, pBytes, 4, NULL, NULL);
            }

            if (FALSE == bRet) { break; }

            nOffset += nWriteSize;
        } while (nOffset < nCompressedSize);
    }

    if (NULL != pColor) { HANWinHeapFree(hHeap, 0, pColor); }
    if (NULL != pCompress) { HANWinHeapFree(hHeap, 0, pCompress); }

    return bRet;
}
static BOOL SavePngChunk_IEND(HANDLE hFile)
{
    HANSIZE nOffset = 0;
    uint32_t cCrc;
    uint8_t pData[12];
    
    nOffset = 0;
    WritePngData4ByteMSB(&pData[nOffset], 0); nOffset += 4;
    memcpy(&pData[nOffset], "IEND", 4);
    cCrc = CRC32_STD(&pData[nOffset], 4); nOffset += 4;
    WritePngData4ByteMSB(&pData[nOffset], cCrc);

    return WriteFile(hFile, pData, sizeof(pData), NULL, NULL);
}

static void InitIDATColorData(uint8_t* pData, HANPCPICTURE pPicture)
{
    PCPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    HANSIZE nRowSize = 1 + (pResolution->pxWidth * 4);
    HANSIZE nOffset;
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;

    for (uint32_t nRow = 0; nRow < pResolution->pxHeight; nRow++)
    {
        nOffset = nRow * nRowSize;
        pData[nOffset] = 0;
        nOffset++;
        for (uint32_t nCol = 0; nCol < pResolution->pxWidth; nCol++)
        {
            pData[nOffset] = pPictureMap[nRow][nCol].r; nOffset++;
            pData[nOffset] = pPictureMap[nRow][nCol].g; nOffset++;
            pData[nOffset] = pPictureMap[nRow][nCol].b; nOffset++;
            pData[nOffset] = pPictureMap[nRow][nCol].a; nOffset++;
        }
    }
}
