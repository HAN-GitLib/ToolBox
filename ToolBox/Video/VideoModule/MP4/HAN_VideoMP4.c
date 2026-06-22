/* 名词解释：
 *      H264规范：Rec. ITU-T H.264
 *      视频规范：ISO/IEC 14496-12
 */

#include <inttypes.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <CommCtrl.h>

#include "HAN_VideoMP4.h"
#include "HAN_VideoMP4Box.h"
#include "..\H264\HAN_VideoH264.h"

typedef enum {
    VIDEO_MP4_BOX_INFO_HEADER_FIELD,
    VIDEO_MP4_BOX_INFO_HEADER_VALUE,
    VIDEO_MP4_BOX_INFO_HEADER_CNT,
} VIDEOMP4BOXINFOHEADER;

typedef struct tagVIDEOMP4BOXTREE {
    HTREEITEM                       hItem;
    VIDEOMP4BOX                     mp4Box;
    struct tagVIDEOMP4BOXTREE*      pParent;
} VIDEOMP4BOXTREE, * PVIDEOMP4BOXTREE;

typedef struct tagVIDEOMP4READBOXMACHINE {
    const uint8_t*                  pData;
    HANSIZE                         nLen;
    HANPSIZE                        pBoxCnt;
    PVIDEOMP4BOXTREE                pTree;
} VIDEOMP4READBOXMACHINE, * PVIDEOMP4READBOXMACHINE;

typedef struct tagVIDEOMP4WNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    VIDEOCREATEPARAM                paramVideo;
    struct {
        HWND                        hTree;
        HWND                        hInfo;
        struct {
            HWND                    hText;
            HWND                    hList;
        } trackId;
        struct {
            BOOL                    bLock;
            HWND                    hText;
            HWND                    hInput;
            HWND                    hUpDown;
        } sampleId;
        HTREEITEM                   hFileTree;
        VIDEOMP4BOXINFO             boxInfo;
        struct {
            HANSIZE                 nCnt;
            PVIDEOMP4BOXTREE        pBoxTree;
            PVIDEOMP4BOXTREE        pChoosenBox;
        } map;
    } box;
    struct {
        HANSIZE                     nSPSCnt;
        HANSIZE                     nPPSCnt;
        HANSIZE                     nSampleCnt;
    } size;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
    uint8_t*                        pBufPtr;
    uint8_t                         pBuf[];
} VIDEOMP4WNDEXTRA, *PVIDEOMP4WNDEXTRA;

typedef struct tagVIDEOMP4BOXTYPEINFO {
    PCHAR                           pType;
    HANPSTR                         (*GetBoxName)(void);
    HANSIZE                         nReadSubBoxOffset;
    void                            (*UpdateBoxInfoWindow)(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);   // 可以填 NULL 来填充纯数据
} VIDEOMP4BOXTYPEINFO;

typedef struct tagVIDEOMP4BOXTRACKHANDLERINFO {
    uint8_t                         pType[5];
} VIDEOMP4BOXTRACKHANDLERINFO;

typedef struct tagVIDEOMP4BOXPROFILEIDCINFO {
    HANPSTR                         pName;
} VIDEOMP4BOXPROFILEIDCINFO;

static LRESULT CALLBACK VideoMP4WndProc(HWND hVideoMP4, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hVideoMP4, LPARAM lParam);
static void CommandCallback(PVIDEOMP4WNDEXTRA mp4Info, WPARAM wParam);
static LRESULT NotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify);
static void DestroyCallback(PVIDEOMP4WNDEXTRA mp4Info);
static void InitMP4BoxInfo(PVIDEOMP4WNDEXTRA mp4Info);
static void GetMP4BoxInfoWindowPos(PVIDEOMP4WNDEXTRA mp4Info, PRECT pPos, VIDEOMP4BOXTYPE boxType);
static void InitMP4BoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info);
static void InitMP4BoxTreeWindow(PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE MP4Process(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE ReadMP4Box(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE ReadMP4Boxes(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxTreeMap(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static void PrintMP4Text4Bytes(HANCHAR pDest[5], const uint8_t pSrc[4]);
static VIDEOMP4BOXTYPE GetMP4BoxType(const uint8_t* pType);
static PVIDEOMP4WNDEXTRA ReallocMP4InfoMemory(PVIDEOMP4WNDEXTRA mp4Info);
static void VideoMP4PrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen);
static LRESULT BoxTreeNotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify);
static void UpdateBoxInfoPos(PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoBeforeSubBoxes(PCVIDEOMP4BOX pBox, PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateBoxInfoAfterSubBoxes(PCVIDEOMP4BOX pBox, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info, TVITEM* pItem);
static void InitTrackIdListWindow(PVIDEOMP4WNDEXTRA mp4Info);
static void ChooseBoxInfoTrackCallback(PVIDEOMP4WNDEXTRA mp4Info);
static void ChooseBoxInfoSampleCallback(PVIDEOMP4WNDEXTRA mp4Info);

static inline HANINT GetBoxInfoWindowWidth(void);
static inline uint16_t ReadMP4Data2ByteMSB(const uint8_t pData[2]);
static inline uint32_t ReadMP4Data3ByteMSB(const uint8_t pData[3]);
static inline uint32_t ReadMP4Data4ByteMSB(const uint8_t pData[4]);

static void UpdateMP4InfoBeforeSubBoxes_trak(PVIDEOMP4WNDEXTRA mp4Info);

static BOOL UpdateMP4InfoAfterSubBoxes_trak(PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateMP4InfoAfterSubBoxes_avcC(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4InfoAfterSubBoxes_stts(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4InfoAfterSubBoxes_ctts(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4InfoAfterSubBoxes_stss(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4InfoAfterSubBoxes_stsc(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4InfoAfterSubBoxes_stsz(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4InfoAfterSubBoxes_stco64(PVIDEOMP4WNDEXTRA mp4Info);

static HANINT UpdateBoxInfoWindow_InsertLine(HANPSTR pField, HANPSTR pValue, HANINT nId, HWND hListView);
static HANINT UpdateBoxInfoWindow_FullBoxVersionFlags(HANINT nId, HWND hListView, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB);
static HANINT UpdateBoxInfoWindow_BlankBox(const uint8_t* pData, HANSIZE nLen, HANINT nId, HWND hListView);
static void UpdateBoxInfoWindow_PrintTimeDuration(HANPSTR pText, HANSIZE nLen, PULARGE_INTEGER pTimeDuration);
static void UpdateBoxInfoWindow_SetTitle(VIDEOMP4BOXTYPE eType, HWND hListView);
static void UpdateBoxInfoWindow_Default(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_ftyp(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_free(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_mdat(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_mvhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_iods(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_tkhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_elst(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_mdhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_hdlr(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_vmhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_dref(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_url_(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_stsd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_avc1(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_avcC(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_pasp(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_btrt(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_stts(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_ctts(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_stss(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_stsc(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_stsz(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_stco(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_co64(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_smhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_mp4a(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_meta(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow_data(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info);

static void UpdateBoxInfoWindow_mdatSample(HANSIZE nTrackId, HANSIZE nSampleId, PVIDEOMP4WNDEXTRA mp4Info);

static HANINT UpdateH264InfoWindow_SPS(PCVIDEOH264PARAMETER_seq_parameter_set pSPS, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_VUIParam(PCVIDEOH264PARAMETER_vui_parameters pVUI, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_HRDParam(PCVIDEOH264PARAMETER_hrd_parameters pHRD, HANPCSTR pHeader, HWND hListView, HANINT nStartId);

static HANINT UpdateH264InfoWindow_PPS(PCVIDEOH264PARAMETER_pic_parameter_set pPPS, HWND hListView, HANINT nStartId);

static HANINT UpdateH264InfoWindow_Slice(PCVIDEOH264PARAMETER_slice_layer_without_partitioning pSlice, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_RefPicListMVCModification(PCVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_RefPicListModification(PCVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_DecRefPicMarking(PCVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, HWND hListView, HANINT nStartId);

static HANSIZE DecodeBoxDataReadDataByVersion(const uint8_t* pData, uint8_t nVersion, PULARGE_INTEGER pLargeInt);
static HANSIZE DecodeBoxDataDateTime(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFODATETIME pDateTime);
static void DecodeBoxData_FullBoxVersionFlags(const uint8_t* pData, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB);
static BOOL DecodeBoxData_mvhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mvhd pmvhd);
static BOOL DecodeBoxData_tkhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_tkhd ptkhd);
static HANSIZE DecodeBoxData_elst(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFO_elst pelst);
static BOOL DecodeBoxData_mdhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mdhd pmdhd);
static void DecodeBoxData_hdlr(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_hdlr phdlr);
static void DecodeBoxData_vmhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_vmhd pvmhd);
static VIDEOMP4TRACKHANDLERTYPE DecodeBoxDataGetTrackHandlerType(uint8_t pType[4]);
static BOOL DecodeBoxData_url_(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_url_ purl_);
static void DecodeBoxData_avc1(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avc1 pavc1);
static void DecodeBoxData_avcC(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avcC pavcC);
static void DecodeBoxData_stsdSubBoxDefault(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsdSubBoxDefault pDef);
static void DecodeBoxData_stsz(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsz pstsz);
static void DecodeBoxData_stco(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pstco);
static void DecodeBoxData_co64(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pco64);
static void DecodeBoxData_mp4a(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mp4a pmp4a);

static BOOL DecodeMP4(PVIDEOMP4WNDEXTRA mp4Info, HANSIZE nFrame);

static const HANINT sg_pMP4BoxInfoHeaderWidth[VIDEO_MP4_BOX_INFO_HEADER_CNT] = {
    [VIDEO_MP4_BOX_INFO_HEADER_FIELD] = 150,
    [VIDEO_MP4_BOX_INFO_HEADER_VALUE] = 300,
};
static const VIDEOMP4BOXTRACKHANDLERINFO sg_pMP4BoxTrackHandlerInfo[VIDEO_MP4_TRACK_HANDLER_TYPE_CNT] = {
    [VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO] = {
        .pType = "vide",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_SOUND] = {
        .pType = "soun",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_SUBTITLE] = {
        .pType = "subt",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_HINT] = {
        .pType = "hint",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_METADATA] = {
        .pType = "meta",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_TEXT] = {
        .pType = "text",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_TIMECODE] = {
        .pType = "tmcd",
    },
};
static const VIDEOMP4BOXTYPEINFO sg_pMP4BoxType[VIDEO_MP4_BOX_TYPE_CNT] = {
    [VIDEO_MP4_BOX_TYPE_ftyp] = {
        .pType = "ftyp",
        .GetBoxName = GetMP4_ftyp_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_ftyp,
    },
    [VIDEO_MP4_BOX_TYPE_free] = {
        .pType = "free",
        .GetBoxName = GetMP4_free_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_free,
    },
    [VIDEO_MP4_BOX_TYPE_mdat] = {
        .pType = "mdat",
        .GetBoxName = GetMP4_mdat_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mdat,
    },
    [VIDEO_MP4_BOX_TYPE_moov] = {
        .pType = "moov",
        .GetBoxName = GetMP4_moov_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_mvhd] = {
        .pType = "mvhd",
        .GetBoxName = GetMP4_mvhd_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mvhd,
    },
    [VIDEO_MP4_BOX_TYPE_iods] = {
        .pType = "iods",
        .GetBoxName = GetMP4_iods_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_iods,
    },
    [VIDEO_MP4_BOX_TYPE_trak] = {
        .pType = "trak",
        .GetBoxName = GetMP4_trak_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_tkhd] = {
        .pType = "tkhd",
        .GetBoxName = GetMP4_tkhd_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_tkhd,
    },
    [VIDEO_MP4_BOX_TYPE_edts] = {
        .pType = "edts",
        .GetBoxName = GetMP4_edts_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_elst] = {
        .pType = "elst",
        .GetBoxName = GetMP4_elst_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_elst,
    },
    [VIDEO_MP4_BOX_TYPE_mdia] = {
        .pType = "mdia",
        .GetBoxName = GetMP4_mdia_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_mdhd] = {
        .pType = "mdhd",
        .GetBoxName = GetMP4_mdhd_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mdhd,
    },
    [VIDEO_MP4_BOX_TYPE_hdlr] = {
        .pType = "hdlr",
        .GetBoxName = GetMP4_hdlr_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_hdlr,
    },
    [VIDEO_MP4_BOX_TYPE_minf] = {
        .pType = "minf",
        .GetBoxName = GetMP4_minf_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_vmhd] = {
        .pType = "vmhd",
        .GetBoxName = GetMP4_vmhd_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_vmhd,
    },
    [VIDEO_MP4_BOX_TYPE_dinf] = {
        .pType = "dinf",
        .GetBoxName = GetMP4_dinf_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_dref] = {
        .pType = "dref",
        .GetBoxName = GetMP4_dref_Name,
        .nReadSubBoxOffset = 8,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_dref,
    },
    [VIDEO_MP4_BOX_TYPE_url_] = {
        .pType = "url ",
        .GetBoxName = GetMP4_url__Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_url_,
    },
    [VIDEO_MP4_BOX_TYPE_stbl] = {
        .pType = "stbl",
        .GetBoxName = GetMP4_stbl_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_stsd] = {
        .pType = "stsd",
        .GetBoxName = GetMP4_stsd_Name,
        .nReadSubBoxOffset = 8,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsd,
    },
    [VIDEO_MP4_BOX_TYPE_avc1] = {
        .pType = "avc1",
        .GetBoxName = GetMP4_avc1_Name,
        .nReadSubBoxOffset = 78,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_avc1,
    },
    [VIDEO_MP4_BOX_TYPE_avcC] = {
        .pType = "avcC",
        .GetBoxName = GetMP4_avcC_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_avcC,
    },
    [VIDEO_MP4_BOX_TYPE_pasp] = {
        .pType = "pasp",
        .GetBoxName = GetMP4_pasp_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_pasp,
    },
    [VIDEO_MP4_BOX_TYPE_btrt] = {
        .pType = "btrt",
        .GetBoxName = GetMP4_btrt_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_btrt,
    },
    [VIDEO_MP4_BOX_TYPE_stts] = {
        .pType = "stts",
        .GetBoxName = GetMP4_stts_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stts,
    },
    [VIDEO_MP4_BOX_TYPE_ctts] = {
        .pType = "ctts",
        .GetBoxName = GetMP4_ctts_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_ctts,
    },
    [VIDEO_MP4_BOX_TYPE_stss] = {
        .pType = "stss",
        .GetBoxName = GetMP4_stss_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stss,
    },
    [VIDEO_MP4_BOX_TYPE_stsc] = {
        .pType = "stsc",
        .GetBoxName = GetMP4_stsc_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsc,
    },
    [VIDEO_MP4_BOX_TYPE_stsz] = {
        .pType = "stsz",
        .GetBoxName = GetMP4_stsz_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsz,
    },
    [VIDEO_MP4_BOX_TYPE_stco] = {
        .pType = "stco",
        .GetBoxName = GetMP4_stco_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stco,
    },
    [VIDEO_MP4_BOX_TYPE_co64] = {
        .pType = "co64",
        .GetBoxName = GetMP4_stco_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_co64,
    },
    [VIDEO_MP4_BOX_TYPE_smhd] = {
        .pType = "smhd",
        .GetBoxName = GetMP4_smhd_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_smhd,
    },
    [VIDEO_MP4_BOX_TYPE_mp4a] = {
        .pType = "mp4a",
        .GetBoxName = GetMP4_mp4a_Name,
        .nReadSubBoxOffset = 28,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mp4a,
    },
    [VIDEO_MP4_BOX_TYPE_esds] = {
        .pType = "esds",
        .GetBoxName = GetMP4_esds_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_udta] = {
        .pType = "udta",
        .GetBoxName = GetMP4_udta_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_meta] = {
        .pType = "meta",
        .GetBoxName = GetMP4_meta_Name,
        .nReadSubBoxOffset = 4,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_meta,
    },
    [VIDEO_MP4_BOX_TYPE_ilst] = {
        .pType = "ilst",
        .GetBoxName = GetMP4_ilst_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_data] = {
        .pType = "data",
        .GetBoxName = GetMP4_data_Name,
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_data,
    },
    [VIDEO_MP4_BOX_TYPE_desc] = {
        .pType = "desc",
        .GetBoxName = GetMP4_desc_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_Copyright_too] = {
        .pType = "\xA9too",
        .GetBoxName = GetMP4_Copyright_too_Name,
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
};

BOOL CheckMP4Type(const uint8_t* pData, HANSIZE nLen)
{
    BOOL bRet = FALSE;

    if ((8 <= nLen) && (!memcmp(&pData[4], sg_pMP4BoxType[VIDEO_MP4_BOX_TYPE_ftyp].pType, 4))) { bRet = TRUE; }

    return bRet;
}

void RegisterHANVideoMP4(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = VideoMP4WndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PVIDEOMP4WNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)GetStockObject(VIDEO_BACKGROUND_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_VIDEO_MP4_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);

    INITCOMMONCONTROLSEX icex = {0};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);
}

static LRESULT CALLBACK VideoMP4WndProc(HWND hVideoMP4, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PVIDEOMP4WNDEXTRA mp4Info = (PVIDEOMP4WNDEXTRA)GetWindowLongPtr(hVideoMP4, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hVideoMP4, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(mp4Info, wParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(NULL_BRUSH);
        } break;
        case WM_NOTIFY: {
            lWndProcRet = NotifyCallback(mp4Info, (NMHDR*)lParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(mp4Info);
            lWndProcRet = DefWindowProc(hVideoMP4, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hVideoMP4, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hVideoMP4, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PVIDEOMP4WNDEXTRA mp4Info;
    PVIDEOMP4WNDEXTRA mp4TempInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PVIDEOCREATEPARAM pVideoCreateParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    RECT rcBoxInfoSize;
    HANSIZE nBoxCnt;

    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        mp4Info = (PVIDEOMP4WNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(VIDEOMP4WNDEXTRA));
        if (NULL == mp4Info) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hVideoMP4, 0, (LONG_PTR)mp4Info);

        mp4Info->hHeap = hHeap;
        mp4Info->hInst = hInst;
        mp4Info->hSelf = hVideoMP4;

        InitMP4BoxInfo(mp4Info);

        GetClientRect(hVideoMP4, &rcClientSize);

        mp4Info->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        mp4Info->hFont.hSys = CreateFontIndirect(&g_lfInfoFont);

        nWinX = 0;
        nWinY = 0;
        nWinW = HAN_VIDEO_MP4_BOX_TREE_W;
        nWinH = GetRectH(&rcClientSize);
        mp4Info->box.hTree = CreateWindow(WC_TREEVIEW, TEXT("BOX总览"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_TREE, hInst, NULL);

        nWinX += nWinW + VIDEO_WINDOW_DX;
        nWinY = VIDEO_WINDOW_DY;
        nWinW = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH;
        nWinH = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT;
        mp4Info->box.trackId.hText = CreateWindow(TEXT("static"), TEXT("轨道 ID"),
            WS_CHILD | SS_CENTERIMAGE,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_TRACK_ID_TEXT, hInst, NULL);
        nWinX += nWinW;
        nWinW = GetBoxInfoWindowWidth() - nWinW;
        nWinH = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_TRACK_LIST_HEIGHT;
        mp4Info->box.trackId.hList = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_TRACK_ID_LIST, hInst, NULL);

        nWinX -= HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH;
        nWinY += HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT + VIDEO_WINDOW_DY;
        nWinW = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH;
        nWinH = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT;
        mp4Info->box.sampleId.hText = CreateWindow(TEXT("static"), TEXT("样本索引"),
            WS_CHILD | SS_CENTERIMAGE,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_SAMPLE_ID_TEXT, hInst, NULL);
        nWinX += nWinW;
        nWinW = GetBoxInfoWindowWidth() - nWinW;
        mp4Info->box.sampleId.hInput = CreateWindow(TEXT("edit"), TEXT("0"),
            WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_SAMPLE_ID_INPUT, hInst, NULL);
        mp4Info->box.sampleId.hUpDown = CreateWindow(UPDOWN_CLASS, TEXT(""),
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_NOTHOUSANDS,
            0, 0, 0, 0,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_SAMPLE_ID_UPDOWN, hInst, NULL);

        GetMP4BoxInfoWindowPos(mp4Info, &rcBoxInfoSize, VIDEO_MP4_BOX_TYPE_CNT);
        mp4Info->box.hInfo = CreateWindow(WC_LISTVIEW, TEXT("BOX信息"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            rcBoxInfoSize.left, rcBoxInfoSize.top, GetRectW(&rcBoxInfoSize), GetRectH(&rcBoxInfoSize),
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_INFO, hInst, NULL);

        SendMessage(mp4Info->box.trackId.hText, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);
        SendMessage(mp4Info->box.trackId.hList, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);
        SendMessage(mp4Info->box.sampleId.hText, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);
        SendMessage(mp4Info->box.sampleId.hInput, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);

        InitMP4BoxInfoWindow(mp4Info);
    }
    /* 解码文件的数据段，填充信息 */
    if (-1 != lWndProcRet)
    {
        mp4Info->paramVideo = *pVideoCreateParam;
        nBoxCnt = MP4Process(mp4Info->paramVideo.pData, mp4Info->paramVideo.nLen, mp4Info);
        mp4Info->box.map.nCnt = nBoxCnt;
        if (0 == nBoxCnt) { lWndProcRet = -1; }
    }
    /* 重新分配 mp4Info 内存 */
    if (-1 != lWndProcRet)
    {
        mp4TempInfo = ReallocMP4InfoMemory(mp4Info);
        if (NULL != mp4TempInfo)
        {
            mp4Info = mp4TempInfo;
            SetWindowLongPtr(hVideoMP4, 0, (LONG_PTR)mp4Info);
        }
        else { lWndProcRet = -1; }
    }
    /* 视频解码 */
    if (-1 != lWndProcRet)
    {
        BOOL bDecodeRet = TRUE;

        InitMP4BoxTreeWindow(mp4Info);
        MP4Process(pVideoCreateParam->pData, mp4Info->paramVideo.nLen, mp4Info);
        InitTrackIdListWindow(mp4Info);
        bDecodeRet = DecodeMP4(mp4Info, 0);

        (void)bDecodeRet;
    }

    return lWndProcRet;
}
static void CommandCallback(PVIDEOMP4WNDEXTRA mp4Info, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_VIDEO_MP4_BOX_TRACK_ID_LIST: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { ChooseBoxInfoTrackCallback(mp4Info); }
        } break;
        case WID_VIDEO_MP4_BOX_SAMPLE_ID_INPUT: {
            if ((FALSE == mp4Info->box.sampleId.bLock) && (EN_CHANGE == HIWORD(wParam))) { ChooseBoxInfoSampleCallback(mp4Info); }
        } break;

        default: { } break;
    }
}
static LRESULT NotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->idFrom) {
        case WID_VIDEO_MP4_BOX_TREE: {
            lWndProcRet = BoxTreeNotifyCallback(mp4Info, pNotify);
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void DestroyCallback(PVIDEOMP4WNDEXTRA mp4Info)
{
    HANWinHeapFree(mp4Info->hHeap, 0, mp4Info);
}
static void InitMP4BoxInfo(PVIDEOMP4WNDEXTRA mp4Info)
{
    mp4Info->box.map.pBoxTree = NULL;
    mp4Info->box.map.pChoosenBox = NULL;
    mp4Info->box.sampleId.bLock = FALSE;
    mp4Info->box.boxInfo.track.nCnt = 0;
    mp4Info->box.boxInfo.track.nTargetId = 0;
    mp4Info->size.nSPSCnt = 0;
    mp4Info->size.nPPSCnt = 0;
    mp4Info->size.nSampleCnt = 0;
    mp4Info->pBufPtr = NULL;
}
static void GetMP4BoxInfoWindowPos(PVIDEOMP4WNDEXTRA mp4Info, PRECT pPos, VIDEOMP4BOXTYPE boxType)
{
    RECT rcClientSize;

    GetClientRect(mp4Info->hSelf, &rcClientSize);
    pPos->left = HAN_VIDEO_MP4_BOX_TREE_W + VIDEO_WINDOW_DX;
    pPos->right = pPos->left + GetBoxInfoWindowWidth();
    pPos->bottom = GetRectH(&rcClientSize);

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_mdat: {
            pPos->top = (HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT * 2) + (VIDEO_WINDOW_DY * 3);
        } break;
        default: { pPos->top = 0; } break;
    }
}
static void InitMP4BoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };
    
    SendMessage(mp4Info->box.sampleId.hUpDown, UDM_SETBUDDY, (WPARAM)(mp4Info->box.sampleId.hInput), 0);
    ListView_SetExtendedListViewStyle(mp4Info->box.hInfo, LVS_EX_FULLROWSELECT);
    
    for (VIDEOMP4BOXINFOHEADER iLoop = 0; iLoop < VIDEO_MP4_BOX_INFO_HEADER_CNT; iLoop++)
    {
        lvTitle.iSubItem = iLoop;
        lvTitle.pszText = TEXT("");
        lvTitle.cx = sg_pMP4BoxInfoHeaderWidth[iLoop];
        lvTitle.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(mp4Info->box.hInfo, iLoop, &lvTitle);
    }
}
static void InitMP4BoxTreeWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    HANPSTR pFileName = PathFindFileName(mp4Info->paramVideo.pFileName);
    TVINSERTSTRUCT tviInsert;

    tviInsert.hParent = NULL;
    tviInsert.hInsertAfter = TVI_FIRST;
    tviInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
    tviInsert.item.pszText = pFileName;
    tviInsert.item.cchTextMax = (HANINT)HAN_strlen(pFileName) + 1;
    tviInsert.item.lParam = (LPARAM)NULL;
    mp4Info->box.hFileTree = TreeView_InsertItem(mp4Info->box.hTree, &tviInsert);
}
static HANSIZE MP4Process(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANSIZE nBoxCnt = 0;
    VIDEOMP4READBOXMACHINE readBox = {
        .pData = pData,
        .nLen = nLen,
        .pBoxCnt = &nBoxCnt,
        .pTree = NULL,
    };

    if (0 == ReadMP4Boxes(&readBox, mp4Info)) { nBoxCnt = 0; }

    return nBoxCnt;
}
static HANSIZE ReadMP4Box(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANSIZE nRet = 0;
    const uint8_t* pData = pReadBox->pData;
    VIDEOMP4READBOXMACHINE readSubBox = *pReadBox;
    VIDEOMP4BOX mp4Box;
    PVIDEOMP4BOXTREE pTree;
    BOOL bOk = TRUE;
    HANSIZE nDataOffset = 8;
    ULARGE_INTEGER liSize;
    HANSIZE nReadSubBoxOffset;

    mp4Box.nSize = ReadMP4Data4ByteMSB(pData);
    switch (mp4Box.nSize) {
        case 0: { mp4Box.nSize = pReadBox->nLen; } break;
        case 1: {
            liSize.HighPart = ReadMP4Data4ByteMSB(&pData[8]);
            liSize.LowPart = ReadMP4Data4ByteMSB(&pData[12]);
            mp4Box.nSize = liSize.QuadPart;
            nDataOffset = 16;
        } break;
        default: { } break;
    }
    if (pReadBox->nLen < mp4Box.nSize) { bOk = FALSE; }

    if (TRUE == bOk) /* 先处理好当前节点，准备好给子 box 做递归 */
    {
        memcpy(mp4Box.pType, &pData[4], 4);
        mp4Box.eType = GetMP4BoxType(mp4Box.pType);
        mp4Box.nDataLen = mp4Box.nSize - nDataOffset;
        mp4Box.pData = &pData[nDataOffset];

        if (NULL != mp4Info->box.map.pBoxTree)
        {
            pTree = &(mp4Info->box.map.pBoxTree)[*(pReadBox->pBoxCnt)];
            pTree->mp4Box = mp4Box;
            pTree->pParent = pReadBox->pTree;

            readSubBox.pTree = pTree;
            UpdateBoxTreeMap(&readSubBox, mp4Info);
        }
        (*(pReadBox->pBoxCnt))++;
        
        if (mp4Box.eType < VIDEO_MP4_BOX_TYPE_CNT)
        {
            /* 尝试解析子 BOX，把数据段传下去 */
            if (0 <= mp4Box.eType)
            {
                UpdateBoxInfoBeforeSubBoxes(&mp4Box, mp4Info);
                nReadSubBoxOffset = sg_pMP4BoxType[mp4Box.eType].nReadSubBoxOffset;
                if (HAN_VIDEO_MP4_INVALID_SIZE != nReadSubBoxOffset)
                {
                    readSubBox.pData = &pData[8 + nReadSubBoxOffset];
                    readSubBox.nLen = mp4Box.nDataLen - nReadSubBoxOffset;
                    readSubBox.pBoxCnt = pReadBox->pBoxCnt;
                    ReadMP4Boxes(&readSubBox, mp4Info);
                }
                bOk = UpdateBoxInfoAfterSubBoxes(&mp4Box, mp4Info);
            }
        }
    }

    if (TRUE == bOk) { nRet = mp4Box.nSize; }

    return nRet;
}
static HANSIZE ReadMP4Boxes(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANSIZE nRet = 0;
    const uint8_t* pData = pReadBox->pData;
    HANSIZE nLen = pReadBox->nLen;
    VIDEOMP4READBOXMACHINE readSubBox = *pReadBox;
    HANSIZE nOffset;
    HANSIZE nBoxLen;

    nOffset = 0;
    while (nOffset < nLen)
    {
        nBoxLen = ReadMP4Box(&readSubBox, mp4Info);
        if (0 < nBoxLen)
        {
            nOffset += nBoxLen;
            readSubBox.pData = &pData[nOffset];
            readSubBox.nLen = nLen - nOffset;
            nRet += nBoxLen;
        }
        else
        {
            nRet = 0;
            break;
        }
    }

    return nRet;
}
static void UpdateBoxTreeMap(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANCHAR pText[5];
    PVIDEOMP4BOXTREE pTree = pReadBox->pTree;
    PVIDEOMP4BOXTREE pParent = pReadBox->pTree->pParent;
    TVINSERTSTRUCT tviInsert;

    PrintMP4Text4Bytes(pText, pTree->mp4Box.pType);

    if (NULL != pParent) { tviInsert.hParent = pParent->hItem; }
    else { tviInsert.hParent = mp4Info->box.hFileTree; }
    tviInsert.hInsertAfter = TVI_LAST;
    tviInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
    tviInsert.item.pszText = pText;
    tviInsert.item.cchTextMax = ArrLen(pText);
    tviInsert.item.lParam = (LPARAM)pTree;
    pReadBox->pTree->hItem = TreeView_InsertItem(mp4Info->box.hTree, &tviInsert);
}
static void PrintMP4Text4Bytes(HANCHAR pDest[5], const uint8_t pSrc[4])
{
    CHAR pMultiBytes[5] = { pSrc[0], pSrc[1], pSrc[2], pSrc[3], '\0', };
    HAN_snprintf(pDest, 5, HANPSTR_PRINT_PCHAR_FORMAT, pMultiBytes);
}
static VIDEOMP4BOXTYPE GetMP4BoxType(const uint8_t* pType)
{
    VIDEOMP4BOXTYPE eRet = VIDEO_MP4_BOX_TYPE_CNT;

    for (VIDEOMP4BOXTYPE eType = 0; eType < VIDEO_MP4_BOX_TYPE_CNT; eType++)
    {
        if (!memcmp(sg_pMP4BoxType[eType].pType, pType, 4))
        {
            eRet = eType;
        }
    }

    return eRet;
}
static PVIDEOMP4WNDEXTRA ReallocMP4InfoMemory(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4WNDEXTRA pRet;
    PVIDEOMP4BOXINFO pBoxInfo = &(mp4Info->box.boxInfo);
    HANSIZE nBoxCnt = mp4Info->box.map.nCnt;
    SIZE_T nOffset;
    SIZE_T nBoxTreeSize;
    SIZE_T nTrackSize;
    SIZE_T nSPSSize;
    SIZE_T nPPSSize;
    SIZE_T nSampleSize;

    nBoxTreeSize = sizeof(VIDEOMP4BOXTREE) * nBoxCnt;
    nTrackSize = sizeof(VIDEOMP4TRACK) * pBoxInfo->track.nCnt;
    nSPSSize = sizeof(VIDEOH264PARAMETER_seq_parameter_set) * mp4Info->size.nSPSCnt;
    nPPSSize = sizeof(VIDEOH264PARAMETER_pic_parameter_set) * mp4Info->size.nSPSCnt;
    nSampleSize = sizeof(VIDEOMP4DECODEINFOSAMPLE) * mp4Info->size.nSampleCnt;
    pRet = (PVIDEOMP4WNDEXTRA)HANWinHeapAlloc(
        mp4Info->hHeap, mp4Info,
        sizeof(VIDEOMP4WNDEXTRA)
         + nBoxTreeSize
         + nTrackSize
         + nSPSSize
         + nPPSSize
         + nSampleSize
    );

    if (NULL != pRet)
    {
        nOffset = 0;
        pRet->box.map.pBoxTree = (PVIDEOMP4BOXTREE)&(pRet->pBuf[nOffset]); nOffset += nBoxTreeSize;
        pRet->box.boxInfo.track.pList = (PVIDEOMP4TRACK)&(pRet->pBuf[nOffset]); nOffset += nTrackSize;

        pRet->pBufPtr = &(pRet->pBuf[nOffset]);
    }

    return pRet;
}
static void VideoMP4PrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen)
{
    HANSIZE nLoopLen = VIDEO_HEX_PRINT_LEN;
    HANSIZE nOffset = 0;

    HAN_strcpy(pText, TEXT(""));
    if (nDataLen < VIDEO_HEX_PRINT_LEN) { nLoopLen = nDataLen; }
    for (HANSIZE iLoop = 0; iLoop < nLoopLen; iLoop++)
    {
        HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("%02X "), pData[iLoop]);
        nOffset += HAN_strlen(&pText[nOffset]);
    }
    if (VIDEO_HEX_PRINT_LEN < nDataLen) { HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("...")); }
}
static LRESULT BoxTreeNotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->code) {
        case TVN_SELCHANGED: {
            NMTREEVIEW* pTreeView = (NMTREEVIEW*)pNotify;
            UpdateBoxInfoWindow(mp4Info, &(pTreeView->itemNew));
            UpdateBoxInfoPos(mp4Info);
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void UpdateBoxInfoPos(PVIDEOMP4WNDEXTRA mp4Info)
{
    RECT rcBoxInfoPos;
    PVIDEOMP4BOXTREE pBoxTree = mp4Info->box.map.pChoosenBox;
    VIDEOMP4BOXTYPE boxType;

    if (NULL == pBoxTree) { boxType = VIDEO_MP4_BOX_TYPE_CNT; }
    else { boxType = pBoxTree->mp4Box.eType; }

    GetMP4BoxInfoWindowPos(mp4Info, &rcBoxInfoPos, boxType);
    MoveWindow(mp4Info->box.hInfo, rcBoxInfoPos.left, rcBoxInfoPos.top, GetRectW(&rcBoxInfoPos), GetRectH(&rcBoxInfoPos), TRUE);
    if (VIDEO_MP4_BOX_TYPE_mdat == boxType)
    {
        ShowWindow(mp4Info->box.trackId.hText, SW_SHOW);
        ShowWindow(mp4Info->box.trackId.hList, SW_SHOW);
        ShowWindow(mp4Info->box.sampleId.hText, SW_SHOW);
        ShowWindow(mp4Info->box.sampleId.hInput, SW_SHOW);
    }
    else
    {
        ShowWindow(mp4Info->box.trackId.hText, SW_HIDE);
        ShowWindow(mp4Info->box.trackId.hList, SW_HIDE);
        ShowWindow(mp4Info->box.sampleId.hText, SW_HIDE);
        ShowWindow(mp4Info->box.sampleId.hInput, SW_HIDE);
    }
}
static void UpdateBoxInfoBeforeSubBoxes(PCVIDEOMP4BOX pBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    if (NULL != mp4Info->pBufPtr)
    {
        switch (pBox->eType) {
            case VIDEO_MP4_BOX_TYPE_trak: { UpdateMP4InfoBeforeSubBoxes_trak(mp4Info); } break;
            default: { } break;
        }
    }
}
static BOOL UpdateBoxInfoAfterSubBoxes(PCVIDEOMP4BOX pBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack;

    if (NULL == mp4Info->pBufPtr) // Buf 还未重新分配内存，此时函数功能是统计需要分配的内存信息，如轨道数、样本数等
    {
        switch (pBox->eType) {
            case VIDEO_MP4_BOX_TYPE_trak: { mp4Info->box.boxInfo.track.nCnt++; } break;
            case VIDEO_MP4_BOX_TYPE_avcC: {
                VIDEOMP4BOXINFO_avcC avcCInfo;
                DecodeBoxData_avcC(pBox, &avcCInfo);
                mp4Info->size.nSPSCnt += avcCInfo.sps.nNum;
                mp4Info->size.nPPSCnt += avcCInfo.pps.nNum;
            } break;
            case VIDEO_MP4_BOX_TYPE_stsz: {
                VIDEOMP4BOXINFO_stsz stszInfo;
                DecodeBoxData_stsz(pBox, &stszInfo);
                mp4Info->size.nSampleCnt += stszInfo.nSampleCnt;
            } break;
            default: { } break;
        }
    }
    else // Buf 已重新分配，需要整理指针
    {
        pTrack = &(mp4Info->box.boxInfo.track.pList)[mp4Info->box.boxInfo.track.nTargetId];
        switch (pBox->eType) {
            case VIDEO_MP4_BOX_TYPE_trak: { bRet = UpdateMP4InfoAfterSubBoxes_trak(mp4Info); } break;
            case VIDEO_MP4_BOX_TYPE_tkhd: { DecodeBoxData_tkhd(pBox, &(pTrack->tkhd)); } break;
            case VIDEO_MP4_BOX_TYPE_hdlr: { DecodeBoxData_hdlr(pBox, &(pTrack->hdlr)); } break;
            case VIDEO_MP4_BOX_TYPE_avc1: { DecodeBoxData_avc1(pBox, &(pTrack->avc1)); } break;
            case VIDEO_MP4_BOX_TYPE_avcC: { DecodeBoxData_avcC(pBox, &(pTrack->avcC)); } break;
            case VIDEO_MP4_BOX_TYPE_stts: { DecodeBoxData_stsdSubBoxDefault(pBox, &(pTrack->stts)); } break;
            case VIDEO_MP4_BOX_TYPE_ctts: { DecodeBoxData_stsdSubBoxDefault(pBox, &(pTrack->ctts)); } break;
            case VIDEO_MP4_BOX_TYPE_stss: { DecodeBoxData_stsdSubBoxDefault(pBox, &(pTrack->stss)); } break;
            case VIDEO_MP4_BOX_TYPE_stsc: { DecodeBoxData_stsdSubBoxDefault(pBox, &(pTrack->stsc)); } break;
            case VIDEO_MP4_BOX_TYPE_stsz: { DecodeBoxData_stsz(pBox, &(pTrack->stsz)); } break;
            case VIDEO_MP4_BOX_TYPE_stco: { DecodeBoxData_stco(pBox, &(pTrack->stco64)); } break;
            case VIDEO_MP4_BOX_TYPE_co64: { DecodeBoxData_co64(pBox, &(pTrack->stco64)); } break;
            default: { } break;
        }
    }

    return bRet;
}
static void UpdateBoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info, TVITEM* pItem)
{
    VIDEOMP4BOXTYPE eType = VIDEO_MP4_BOX_TYPE_CNT;
    PVIDEOMP4BOXTREE pBoxTree = (PVIDEOMP4BOXTREE)(pItem->lParam);

    if (((LPARAM)NULL) != pItem->lParam)
    {
        mp4Info->box.map.pChoosenBox = pBoxTree;
        eType = GetMP4BoxType(pBoxTree->mp4Box.pType);

        SendMessage(mp4Info->box.hInfo, WM_SETREDRAW, FALSE, 0);

        ListView_DeleteAllItems(mp4Info->box.hInfo);
        if ((0 <= eType) && (eType < VIDEO_MP4_BOX_TYPE_CNT))
        {
            UpdateBoxInfoWindow_SetTitle(eType, mp4Info->box.hInfo);
            if (NULL != sg_pMP4BoxType[eType].UpdateBoxInfoWindow) { sg_pMP4BoxType[eType].UpdateBoxInfoWindow(pBoxTree, mp4Info); }
            else { (void)UpdateBoxInfoWindow_BlankBox(pBoxTree->mp4Box.pData, pBoxTree->mp4Box.nDataLen, 0, mp4Info->box.hInfo); }
        }
        else { UpdateBoxInfoWindow_Default(pBoxTree, mp4Info->box.hInfo); }

        SendMessage(mp4Info->box.hInfo, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(mp4Info->box.hInfo, NULL, TRUE);
    }
}
static void InitTrackIdListWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4TRACK pTrack = mp4Info->box.boxInfo.track.pList;
    HANSIZE nTrackCnt = mp4Info->box.boxInfo.track.nCnt;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    VIDEOMP4TRACKHANDLERTYPE eType;
    HANINT nCursel = 0;

    for (HANSIZE iLoop = 0; iLoop < nTrackCnt; iLoop++)
    {
        eType = pTrack[iLoop].hdlr.handlerType.eType;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("轨道%u（%s，样本数：" HANSIZE_PRINT_FORMAT "）"),
            pTrack[iLoop].tkhd.nTrackId, GetMP4_hdlr_HandlerTypeName(eType), pTrack[iLoop].nSampleCnt);
        ComboBoxAddString(mp4Info->box.trackId.hList, pText);
        if (VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO == eType) { nCursel = (HANINT)iLoop; }
    }
    ComboBoxSetCursel(mp4Info->box.trackId.hList, nCursel);
    SendMessage(mp4Info->box.sampleId.hUpDown, UDM_SETRANGE32, (WPARAM)0, (LPARAM)(pTrack[nCursel].nSampleCnt - 1));
}
static void ChooseBoxInfoTrackCallback(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4TRACK pTrack = mp4Info->box.boxInfo.track.pList;
    HANINT nTrackId;
    HANSIZE nTrackCnt;
    HANSIZE nSampleCnt;

    if (NULL != pTrack)
    {
        nTrackId = ComboBoxGetCursel(mp4Info->box.trackId.hList);
        nTrackCnt = mp4Info->box.boxInfo.track.nCnt;
        if ((0 <= nTrackId) && (nTrackId < (HANINT)nTrackCnt))
        {
            nSampleCnt = pTrack[nTrackId].nSampleCnt;
            SendMessage(mp4Info->box.sampleId.hUpDown, UDM_SETRANGE32, (WPARAM)0, (LPARAM)(nSampleCnt - 1));
            ChooseBoxInfoSampleCallback(mp4Info);
        }
    }
}
static void ChooseBoxInfoSampleCallback(PVIDEOMP4WNDEXTRA mp4Info)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANINT nTrackId = ComboBoxGetCursel(mp4Info->box.trackId.hList);
    PVIDEOMP4TRACK pTrack = mp4Info->box.boxInfo.track.pList;
    HANSIZE nSampleCnt;
    HANSIZE idSample;
    HANSIZE nTrackCnt;
    DWORD nTextCursel;

    if (NULL != pTrack)
    {
        nTrackCnt = mp4Info->box.boxInfo.track.nCnt;
        if ((0 <= nTrackId) && (nTrackId < (HANINT)nTrackCnt))
        {
            mp4Info->box.sampleId.bLock = TRUE;

            pTrack = &pTrack[nTrackId];
            nSampleCnt = pTrack->nSampleCnt;

            GetWindowText(mp4Info->box.sampleId.hInput, pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
            idSample = HAN_strtoul(pText, NULL, 10);
            if (nSampleCnt <= idSample) { idSample = nSampleCnt - 1; }
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), idSample);
            pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1] = TEXT('\0');
            SendMessage(mp4Info->box.sampleId.hInput, EM_GETSEL, (WPARAM)&nTextCursel, (LPARAM)NULL);
            SetWindowText(mp4Info->box.sampleId.hInput, pText);
            SendMessage(mp4Info->box.sampleId.hInput, EM_SETSEL, nTextCursel, nTextCursel);

            ListView_DeleteAllItems(mp4Info->box.hInfo);
            UpdateBoxInfoWindow_mdatSample(nTrackId, idSample, mp4Info);

            mp4Info->box.sampleId.bLock = FALSE;
        }
    }
}

static inline HANINT GetBoxInfoWindowWidth(void)
{
    HANINT nRet = 0;
    for (VIDEOMP4BOXINFOHEADER iLoop = 0; iLoop < VIDEO_MP4_BOX_INFO_HEADER_CNT; iLoop++)
    {
        nRet += sg_pMP4BoxInfoHeaderWidth[iLoop];
    }
    return nRet;
}
static inline uint16_t ReadMP4Data2ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline uint32_t ReadMP4Data3ByteMSB(const uint8_t pData[3])
{
    return (((uint32_t)pData[0] << 16) + ((uint32_t)pData[1] << 8) + (uint32_t)pData[2]);
}
static inline uint32_t ReadMP4Data4ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}

static void UpdateMP4InfoBeforeSubBoxes_trak(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4TRACK pTrack;
    
    pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    pTrack->stts.pRawData = NULL;
    pTrack->ctts.pRawData = NULL;
    pTrack->stss.pRawData = NULL;
    pTrack->stsc.pRawData = NULL;
    pTrack->stsz.pRawData = NULL;
    pTrack->stco64.pRawData = NULL;
}

static BOOL UpdateMP4InfoAfterSubBoxes_trak(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);

    /* 解码 SPS 和 PPS */
    UpdateMP4InfoAfterSubBoxes_avcC(mp4Info);
    /* 解码样本信息 */
    pTrack->pSample = (PVIDEOMP4DECODEINFOSAMPLE)(mp4Info->pBufPtr);
    pTrack->nSampleCnt = pTrack->stsz.nSampleCnt;

    if ((FALSE == UpdateMP4InfoAfterSubBoxes_stts(mp4Info)) ||
        (FALSE == UpdateMP4InfoAfterSubBoxes_ctts(mp4Info)) ||
        (FALSE == UpdateMP4InfoAfterSubBoxes_stss(mp4Info)) ||
        (FALSE == UpdateMP4InfoAfterSubBoxes_stsc(mp4Info)) ||
        (FALSE == UpdateMP4InfoAfterSubBoxes_stsz(mp4Info)) ||
        (FALSE == UpdateMP4InfoAfterSubBoxes_stco64(mp4Info)))
    {
        bRet = FALSE;
    }

    mp4Info->pBufPtr = (uint8_t*)(&(pTrack->pSample)[pTrack->nSampleCnt]);
    (mp4Info->box.boxInfo.track.nTargetId)++;

    return bRet;
}
static void UpdateMP4InfoAfterSubBoxes_avcC(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    uint8_t nSPSCnt = pTrack->avcC.sps.nNum;
    uint8_t nPPSCnt = pTrack->avcC.pps.nNum;
    const uint8_t* pData;
    uint16_t nLen;
    PVIDEOH264PARAMETER_seq_parameter_set pSPS;
    PVIDEOH264PARAMETER_pic_parameter_set pPPS;
    HANSIZE nOffset;

    nOffset = 0;
    pSPS = (PVIDEOH264PARAMETER_seq_parameter_set)(mp4Info->pBufPtr);
    pData = pTrack->avcC.sps.pList;
    for (uint8_t iLoop = 0; iLoop < nSPSCnt; iLoop++)
    {
        nLen = ReadMP4Data2ByteMSB(&pData[nOffset]);
        DecodeH264Parameter_seq_parameter_set(&pData[nOffset + 2], nLen, &pSPS[iLoop]);
        nOffset += nLen + 2;
    }
    pTrack->avcC.sps.pSPS = pSPS;
    mp4Info->pBufPtr = (uint8_t*)(&pSPS[nSPSCnt]);

    nOffset = 0;
    pPPS = (PVIDEOH264PARAMETER_pic_parameter_set)(mp4Info->pBufPtr);
    pData = pTrack->avcC.pps.pList;
    for (uint8_t iLoop = 0; iLoop < nPPSCnt; iLoop++)
    {
        nLen = ReadMP4Data2ByteMSB(&pData[nOffset]);
        DecodeH264Parameter_pic_parameter_set(&pData[nOffset + 2], nLen, &pPPS[iLoop], NULL);
        nOffset += nLen + 2;
    }
    pTrack->avcC.pps.pPPS = pPPS;
    mp4Info->pBufPtr = (uint8_t*)(&pPPS[nPPSCnt]);
}
static BOOL UpdateMP4InfoAfterSubBoxes_stts(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->stts.pEntry;
    HANSIZE nEntrySize = pTrack->stts.nEntrySize;
    HANSIZE nRecordCnt = pTrack->stts.nCnt;
    HANSIZE idSample = 0;
    HANSIZE nOffset = 0;
    HANSIZE timeDTS = 0;
    uint32_t timeCnt;
    uint32_t timeDelta;

    for (HANSIZE iLoop = 0; iLoop < nRecordCnt; iLoop++)
    {
        if (nEntrySize < (nOffset + 8)) { bRet = FALSE; }
        {
            timeCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            timeDelta = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            for (uint32_t jLoop = 0; jLoop < timeCnt; jLoop++)
            {
                if (idSample < nSampleCnt)
                {
                    pSample[idSample].timeDTS = timeDTS;
                    pSample[idSample].timeDuration = timeDelta;
                    timeDTS += timeDelta;
                    idSample++;
                }
                else
                {
                    bRet = FALSE;
                    break;
                }
            }
        }
        if (FALSE == bRet) { break; }
    }

    if (idSample != pTrack->stsz.nSampleCnt) { bRet = FALSE; }
    
    return bRet;
}
static BOOL UpdateMP4InfoAfterSubBoxes_ctts(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->ctts.pEntry;
    uint8_t nVersion = pTrack->ctts.fbVF.nVersion;
    HANSIZE nEntrySize = pTrack->ctts.nEntrySize;
    HANSIZE nRecordCnt = pTrack->ctts.nCnt;
    HANSIZE idSample = 0;
    HANSIZE nOffset = 0;
    uint32_t timeCnt;
    ULARGE_INTEGER timeOffset;

    if (NULL != pTrack->ctts.pRawData) /* 存在有效的 ctts BOX */
    {
        for (HANSIZE iLoop = 0; iLoop < nRecordCnt; iLoop++)
        {
            if (nEntrySize < (nOffset + 8)) { bRet = FALSE; }
            else
            {
                timeCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
                nOffset += DecodeBoxDataReadDataByVersion(&pEntry[nOffset], nVersion, &timeOffset);
                for (uint32_t jLoop = 0; jLoop < timeCnt; jLoop++)
                {
                    if (idSample < nSampleCnt)
                    {
                        pSample[idSample].timePTS = pSample[idSample].timeDTS + timeOffset.QuadPart;
                        idSample++;
                    }
                    else
                    {
                        bRet = FALSE;
                        break;
                    }
                }
            }
            if (FALSE == bRet) { break; }
        }
    }
    else
    {
        idSample = nSampleCnt;
        for (HANSIZE iLoop = 0; iLoop < idSample; iLoop++)
        {
            pSample[iLoop].timePTS = pSample[iLoop].timeDTS;
        }
    }

    if (idSample != nSampleCnt) { bRet = FALSE; }

    return bRet;
}
static BOOL UpdateMP4InfoAfterSubBoxes_stss(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->stss.pEntry;
    HANSIZE nEntrySize = pTrack->ctts.nEntrySize;
    HANSIZE nRecordCnt;
    HANSIZE nOffset = 0;
    HANSIZE idKeyFrame;
    uint32_t nPrevKeyFrame;
    uint32_t nNextKeyFrame;
    uint32_t idPrevKeyFrame;
    uint32_t idNextKeyFrame = 0;

    if (NULL == pTrack->stss.pRawData)
    {
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            pSample[iLoop].idKeyFrame = iLoop;
        }
    }
    else
    {
        nRecordCnt = pTrack->stss.nCnt;
        if (0 == nRecordCnt)
        {
            for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
            {
                pSample[iLoop].idKeyFrame = HAN_VIDEO_MP4_INVALID_SIZE;
            }
        }
        else
        {
            nPrevKeyFrame = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            if ((0 == nPrevKeyFrame) || (nSampleCnt < nPrevKeyFrame)) { bRet = FALSE; }
            else
            {
                // 0 ~ 第一个 ID
                idPrevKeyFrame = nPrevKeyFrame - 1;
                for (HANSIZE iLoop = 0; iLoop < idPrevKeyFrame; iLoop++)
                {
                    pSample[iLoop].idKeyFrame = HAN_VIDEO_MP4_INVALID_SIZE;
                }
                idKeyFrame = idPrevKeyFrame;
                // 第一个 ID ~ 最后一个 ID
                for (HANSIZE iLoop = 1; iLoop < nRecordCnt; iLoop++)
                {
                    if (nEntrySize < (nOffset + 4)) { bRet = FALSE; }
                    if (TRUE == bRet)
                    {
                        nNextKeyFrame = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
                        if ((nSampleCnt < nNextKeyFrame) || (nNextKeyFrame <= nPrevKeyFrame)) { bRet = FALSE; } // 《根据视频规范，章节 8.6.2.1 规定，关键帧 ID 必须递增》
                        else
                        {
                            idPrevKeyFrame = nPrevKeyFrame - 1;
                            idNextKeyFrame = nNextKeyFrame - 1;
                            for (HANSIZE jLoop = idPrevKeyFrame; jLoop < idNextKeyFrame; jLoop++)
                            {
                                pSample[jLoop].idKeyFrame = idKeyFrame;
                            }
                        }
                    }
                    if (FALSE == bRet) { break; }
                    idKeyFrame = idNextKeyFrame;
                    nPrevKeyFrame = nNextKeyFrame;
                }
                if (TRUE == bRet) // 最后一个 ID ~ 尾
                {
                    if (idNextKeyFrame < nSampleCnt)
                    {
                        for (HANSIZE iLoop = idNextKeyFrame; iLoop < nSampleCnt; iLoop++)
                        {
                            pSample[iLoop].idKeyFrame = idKeyFrame;
                        }
                    }
                }
            }
        }
    }

    return bRet;
}
static BOOL UpdateMP4InfoAfterSubBoxes_stsc(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->stsc.pEntry;
    HANSIZE nEntrySize = pTrack->stsc.nEntrySize;
    HANSIZE idSample = 0;
    HANSIZE nOffset = 0;
    uint32_t idChunk = 0;
    uint32_t nRecordCnt = pTrack->stsc.nCnt;
    uint32_t nLoopCnt = nRecordCnt - 1;
    uint32_t nPrevFirstChunk;
    uint32_t nNextFirstChunk;
    uint32_t nPrevSampleCnt;
    uint32_t nNextSampleCnt;
    uint32_t nPrevDsc;
    uint32_t nNextDsc;
    uint32_t idDescription;
    uint32_t nChunkCntInGroup;

    nNextFirstChunk = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
    nNextSampleCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
    nNextDsc = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
    if (1 == nNextFirstChunk) // 《根据视频规范，章节 8.7.4.3 规定，第一个 first_chunk 必须是 1 》
    {
        for (uint32_t iLoop = 0; iLoop < nLoopCnt; iLoop++)
        {
            if (nEntrySize < (nOffset + 12)) { bRet = FALSE; }
            nPrevFirstChunk = nNextFirstChunk;
            nPrevSampleCnt = nNextSampleCnt;
            nPrevDsc = nNextDsc;
            nNextFirstChunk = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            nNextSampleCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            nNextDsc = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            if ((nSampleCnt < nNextFirstChunk) || (nNextFirstChunk <= nPrevFirstChunk)) { bRet = FALSE; }
            if (TRUE == bRet)
            {
                nChunkCntInGroup = nNextFirstChunk - nPrevFirstChunk;
                idDescription = nPrevDsc - 1;
                for (uint32_t jLoop = 0; jLoop < nChunkCntInGroup; jLoop++)
                {
                    for (uint32_t kLoop = 0; kLoop < nPrevSampleCnt; kLoop++)
                    {
                        if (idSample < nSampleCnt)
                        {
                            pSample[idSample].idChunkGroup = iLoop;
                            pSample[idSample].idChunk = idChunk;
                            pSample[idSample].idDescription = idDescription;
                            idSample++;
                        }
                        else
                        {
                            bRet = FALSE;
                            break;
                        }
                    }
                    if (FALSE == bRet) { break; }
                    idChunk++;
                }
            }
        }
        if (TRUE == bRet)
        {
            idDescription = nNextDsc;
            while (idSample < nSampleCnt)
            {
                for (HANSIZE iLoop = 0; iLoop < nNextSampleCnt; iLoop++)
                {
                    if (idSample < nSampleCnt)
                    {
                        pSample[idSample].idChunkGroup = nLoopCnt;
                        pSample[idSample].idChunk = idChunk;
                        pSample[idSample].idDescription = idDescription;
                        idSample++;
                    }
                    else
                    {
                        bRet = FALSE;
                        break;
                    }
                }
                idChunk++;
            }
        }
    }
    else { bRet = FALSE; }

    return bRet;
}
static BOOL UpdateMP4InfoAfterSubBoxes_stsz(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    const uint8_t* pEntry = pTrack->stsz.pEntry;
    HANSIZE nEntrySize = pTrack->stsz.nEntrySize;
    HANSIZE nSampleCnt = pTrack->stsz.nSampleCnt;
    HANSIZE nOffset = 0;

    if (NULL != pEntry)
    {
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            if (nEntrySize < (nOffset + 4))
            {
                bRet = FALSE;
                break;
            }
            pSample[iLoop].nSize = ReadMP4Data4ByteMSB(&pEntry[nOffset]);
            nOffset += 4;
        }
    }
    else
    {
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            pSample[iLoop].nSize = pTrack->stsz.nSampleSize;
        }
    }

    return bRet;
}
static BOOL UpdateMP4InfoAfterSubBoxes_stco64(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    uint8_t nDataSize = pTrack->stco64.nDataSize;
    const uint8_t* pEntry = pTrack->stco64.pEntry;
    HANSIZE nEntrySize = pTrack->stco64.nEntrySize;
    HANSIZE nOffset;
    HANSIZE sOffset;
    uint32_t idChunk = 0;
    ULARGE_INTEGER ulOffset;
    uint8_t nVersion;

    switch (nDataSize) {
        case 4: { nVersion = 0; } break;
        case 8: { nVersion = 1; } break;
        default: { bRet = FALSE; } break;
    }
    if (TRUE == bRet)
    {
        nOffset = DecodeBoxDataReadDataByVersion(pEntry, nVersion, &ulOffset);
        sOffset = ulOffset.QuadPart;
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            if (idChunk != pSample[iLoop].idChunk)
            {
                if (nEntrySize < (nOffset + nDataSize))
                {
                    bRet = FALSE;
                    break;
                }
                nOffset += DecodeBoxDataReadDataByVersion(&pEntry[nOffset], nVersion, &ulOffset);
                sOffset = ulOffset.QuadPart;
                idChunk = pSample[iLoop].idChunk;
            }
            pSample[iLoop].sOffset = sOffset;
            sOffset += pSample[iLoop].nSize;
        }
    }

    return bRet;
}

static HANINT UpdateBoxInfoWindow_InsertLine(HANPSTR pField, HANPSTR pValue, HANINT nId, HWND hListView)
{
    HANINT nRet;
    LVITEM lvItem = { .mask = LVIF_TEXT ,};
    
    lvItem.iItem = nId;
    lvItem.iSubItem = VIDEO_MP4_BOX_INFO_HEADER_FIELD;
    lvItem.pszText = pField;
    nRet = ListView_InsertItem(hListView, &lvItem);
    if (-1 != nRet) { nRet = nId + 1; }
    else { nRet = nId; }

    lvItem.iSubItem = VIDEO_MP4_BOX_INFO_HEADER_VALUE;
    lvItem.pszText = pValue;
    ListView_SetItem(hListView, &lvItem);

    return nRet;
}
static HANINT UpdateBoxInfoWindow_FullBoxVersionFlags(HANINT nId, HWND hListView, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB)
{
    HANINT nRet = nId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), pFB->nVersion, GetMP4BoxVersionName(pFB->nVersion));
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("版本"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), pFB->cFlags);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("标志"), pText, nRet, hListView);

    return nRet;
}
static HANINT UpdateBoxInfoWindow_BlankBox(const uint8_t* pData, HANSIZE nLen, HANINT nId, HWND hListView)
{
    HANINT nRet = nId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    
    /* 长度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nLen);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("长度"), pText, nRet, hListView);
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pData, nLen);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("数据"), pText, nRet, hListView);

    return nRet;
}
static void UpdateBoxInfoWindow_PrintTimeDuration(HANPSTR pText, HANSIZE nLen, PULARGE_INTEGER pTimeDuration)
{
    uint32_t nHour = (uint32_t)(pTimeDuration->QuadPart / 3600);
    uint8_t nMinute = (uint8_t)((pTimeDuration->QuadPart % 3600) / 60);
    uint8_t nSecond = (uint8_t)(pTimeDuration->QuadPart % 60);

    HAN_snprintf(pText, nLen, TEXT("%u:%u:%u"), nHour, nMinute, nSecond);
}
static void UpdateBoxInfoWindow_SetTitle(VIDEOMP4BOXTYPE eType, HWND hListView)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, sg_pMP4BoxType[eType].pType);
    lvTitle.pszText = pText;
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = sg_pMP4BoxType[eType].GetBoxName();
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_VALUE, &lvTitle);
}
static void UpdateBoxInfoWindow_Default(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    PrintMP4Text4Bytes(pText, pBoxTree->mp4Box.pType);
    lvTitle.pszText = pText;
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = TEXT("不支持该数据块");
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_VALUE, &lvTitle);

    (void)UpdateBoxInfoWindow_BlankBox(pBoxTree->mp4Box.pData, pBoxTree->mp4Box.nDataLen, 0, hListView);
}
static void UpdateBoxInfoWindow_ftyp(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANPSTR pCompatibleBrandsName;
    uint32_t nVersion;
    HANINT nId = 0;
    HANINT nNameId;
    
    /* major brand */
    PrintMP4Text4Bytes(pText, &pData[0]);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_MAJOR_BRAND), pText, nId, hListView);
    /* minor version */
    nVersion = ReadMP4Data4ByteMSB(&pData[4]);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%08X"), nVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_MINOR_VERSION), pText, nId, hListView);
    /* compatible brands */
    pCompatibleBrandsName = GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_COMPATIBLE_BRANDS);
    nNameId = 0;
    for (HANSIZE nOffset = 8; nOffset < pBoxTree->mp4Box.nDataLen; nOffset += 4)
    {
        HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%d"), pCompatibleBrandsName, nNameId + 1);
        PrintMP4Text4Bytes(pText, &pData[nOffset]);
        nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
        nNameId++;
    }
}
static void UpdateBoxInfoWindow_free(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    CHAR pBuf[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANINT nId = 0;

    /* 长度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nLen);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(VIDEO_MP4_free_BOX_FIELD_LEN), pText, nId, hListView);
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pData, nLen);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(VIDEO_MP4_free_BOX_FIELD_DATA), pText, nId, hListView);
    /* 文本 */
    if (HAN_VIDEO_MP4_TEXT_BUF_SIZE <= nLen) { nLen = HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1; }
    memcpy(pBuf, pData, nLen);
    pBuf[nLen] = '\0';
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pBuf);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(VIDEO_MP4_free_BOX_FIELD_TEXT), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_mdat(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    (void)pBoxTree;
    UpdateBoxInfoWindow_mdatSample(0, 0, mp4Info);
}
static void UpdateBoxInfoWindow_mvhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_mvhd mvhdInfo;

    if (TRUE == DecodeBoxData_mvhd(&(pBoxTree->mp4Box), &mvhdInfo))
    {
        nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(mvhdInfo.fbVF));
        /* 创建时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mvhdInfo.creationTime.stTime.wYear, mvhdInfo.creationTime.stTime.wMonth, mvhdInfo.creationTime.stTime.wDay,
            mvhdInfo.creationTime.stTime.wHour, mvhdInfo.creationTime.stTime.wMinute, mvhdInfo.creationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_CREATION_TIME), pText, nId, hListView);
        /* 修改时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mvhdInfo.modificationTime.stTime.wYear, mvhdInfo.modificationTime.stTime.wMonth, mvhdInfo.modificationTime.stTime.wDay,
            mvhdInfo.modificationTime.stTime.wHour, mvhdInfo.modificationTime.stTime.wMinute, mvhdInfo.modificationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_MODIFICATION_TIME), pText, nId, hListView);
        /* 时长 */
        UpdateBoxInfoWindow_PrintTimeDuration(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &mvhdInfo.timeDuration.sTimeDuration);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_DURATION), pText, nId, hListView);
        /* 播放速率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), mvhdInfo.nRate.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_RATE), pText, nId, hListView);
        /* 音量 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), mvhdInfo.nVolume.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_VOLUME), pText, nId, hListView);
        /* 视频变换矩阵 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mvhdInfo.pMatrix[0]);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_MATRIX), pText, nId, hListView);
        /* 下一个轨道的 ID */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mvhdInfo.nNextTrackId);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_NEXT_TRACK_ID), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_iods(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &pData[4], pBoxTree->mp4Box.nDataLen - 4);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_iods_FieldName(VIDEO_MP4_iods_BOX_FIELD_DATA), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_tkhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pFlagsName[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_tkhd tkhdInfo;

    if (TRUE == DecodeBoxData_tkhd(&(pBoxTree->mp4Box), &tkhdInfo))
    {
        /* 版本 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), tkhdInfo.fbVF.nVersion, GetMP4BoxVersionName(tkhdInfo.fbVF.nVersion));
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_VERSION), pText, nId, hListView);
        /* 标志 */
        GetMP4_tkhd_FlagsName(tkhdInfo.fbVF.cFlags, pFlagsName, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X（%s）"), tkhdInfo.fbVF.cFlags, pFlagsName);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_FLAGS), pText, nId, hListView);
        /* 创建时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            tkhdInfo.creationTime.stTime.wYear, tkhdInfo.creationTime.stTime.wMonth, tkhdInfo.creationTime.stTime.wDay,
            tkhdInfo.creationTime.stTime.wHour, tkhdInfo.creationTime.stTime.wMinute, tkhdInfo.creationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_CREATION_TIME), pText, nId, hListView);
        /* 修改时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            tkhdInfo.modificationTime.stTime.wYear, tkhdInfo.modificationTime.stTime.wMonth, tkhdInfo.modificationTime.stTime.wDay,
            tkhdInfo.modificationTime.stTime.wHour, tkhdInfo.modificationTime.stTime.wMinute, tkhdInfo.modificationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_MODIFICATION_TIME), pText, nId, hListView);
        /* 轨道ID */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nTrackId);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_TRACK_ID), pText, nId, hListView);
        /* 轨道时长 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), tkhdInfo.nDuration.QuadPart);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_DURATION), pText, nId, hListView);
        /* 视频层 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nLayer);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_LAYER), pText, nId, hListView);
        /* 替代组 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nAlternateGroup);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_ALTERNATE_GROUP), pText, nId, hListView);
        /* 音量 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), tkhdInfo.nVolume.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_VOLUME), pText, nId, hListView);
        /* 变换矩阵 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.pMatrix[0]);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_MATRIX), pText, nId, hListView);
        /* 分辨率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g * %g"), tkhdInfo.nWidth.nPhy, tkhdInfo.nHeight.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_RESOLUTION), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_elst(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    uint32_t nListCnt;
    VIDEOMP4BOXINFO_elst elstInfo;
    HANSIZE nOffset = 0;
    
    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    nId = UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);

    for (uint32_t iLoop = 0; iLoop < nListCnt; iLoop++)
    {
        nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("条目%d"), iLoop + 1);
        nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);

        nOffset += DecodeBoxData_elst(&pData[nOffset], fbVF.nVersion, &elstInfo);
        /* 段时长 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), elstInfo.nSegmentDuration.QuadPart);
        nId = UpdateBoxInfoWindow_InsertLine(TEXT("段时长"), pText, nId, hListView);
        /* 时间点 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), elstInfo.nMediaTime.QuadPart);
        nId = UpdateBoxInfoWindow_InsertLine(TEXT("时间点"), pText, nId, hListView);
        /* 播放速率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), elstInfo.mediaRate.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(TEXT("播放速率"), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_mdhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    CHAR pLanguage[3];
    HANINT nId = 0;
    VIDEOMP4BOXINFO_mdhd mdhdInfo;

    if (TRUE == DecodeBoxData_mdhd(&(pBoxTree->mp4Box), &mdhdInfo))
    {
        nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(mdhdInfo.fbVF));
        /* 创建时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mdhdInfo.creationTime.stTime.wYear, mdhdInfo.creationTime.stTime.wMonth, mdhdInfo.creationTime.stTime.wDay,
            mdhdInfo.creationTime.stTime.wHour, mdhdInfo.creationTime.stTime.wMinute, mdhdInfo.creationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_CREATION_TIME), pText, nId, hListView);
        /* 修改时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mdhdInfo.modificationTime.stTime.wYear, mdhdInfo.modificationTime.stTime.wMonth, mdhdInfo.modificationTime.stTime.wDay,
            mdhdInfo.modificationTime.stTime.wHour, mdhdInfo.modificationTime.stTime.wMinute, mdhdInfo.modificationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_MODIFICATION_TIME), pText, nId, hListView);
        /* 时长 */
        UpdateBoxInfoWindow_PrintTimeDuration(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &mdhdInfo.timeDuration.sTimeDuration);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_DURATION), pText, nId, hListView);
        /* 语言 */
        pLanguage[0] = mdhdInfo.pLanguage[0];
        pLanguage[1] = mdhdInfo.pLanguage[1];
        pLanguage[2] = '\0';
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pLanguage);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_LANGUAGE), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_hdlr(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_hdlr hdlrInfo;

    DecodeBoxData_hdlr(&(pBoxTree->mp4Box), &hdlrInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(hdlrInfo.fbVF));
    /* 处理器类型 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT TEXT("（%s）"),
        hdlrInfo.handlerType.pType,
        GetMP4_hdlr_HandlerTypeName(hdlrInfo.handlerType.eType));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(VIDEO_MP4_hdlr_BOX_FIELD_HANDLER_TYPE), pText, nId, hListView);
    /* 名称 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, hdlrInfo.pName);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(VIDEO_MP4_hdlr_BOX_FIELD_NAME), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_vmhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_vmhd vmhdInfo;

    DecodeBoxData_vmhd(&(pBoxTree->mp4Box), &vmhdInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(vmhdInfo.fbVF));
    /* 处理器类型 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%04X（%s）"), vmhdInfo.cGraphicsMode, GetMP4_vmhd_GraphicsModeName(vmhdInfo.cGraphicsMode));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(VIDEO_MP4_vmhd_BOX_FIELD_GRAPHICS_MODE), pText, nId, hListView);
    /* 名称 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("（%u, %u, %u）"), vmhdInfo.pOpColor[0], vmhdInfo.pOpColor[1], vmhdInfo.pOpColor[2]);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(VIDEO_MP4_vmhd_BOX_FIELD_OP_COLOR), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_dref(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    uint32_t nListCnt;
    HANSIZE nOffset = 0;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    nId = UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_url_(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_url_ urlInfo;

    if (TRUE == DecodeBoxData_url_(&(pBoxTree->mp4Box), &urlInfo))
    {
        /* 版本 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), urlInfo.fbVF.nVersion, GetMP4BoxVersionName(urlInfo.fbVF.nVersion));
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(VIDEO_MP4_url__BOX_FIELD_VERSION), pText, nId, hListView);
        /* 标志 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X（%s）"), urlInfo.fbVF.cFlags, GetMP4_url__FlagsName(urlInfo.fbVF.cFlags));
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(VIDEO_MP4_url__BOX_FIELD_FLAGS), pText, nId, hListView);
        /* URL */
        if (0x000000 == urlInfo.fbVF.cFlags) { HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, urlInfo.pUrl); }
        else { HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("本文件")); }
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(VIDEO_MP4_url__BOX_FIELD_LOCATION), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_stsd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    uint32_t nListCnt;
    HANSIZE nOffset = 0;
    
    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    nId = UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_avc1(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_avc1 avc1Info;

    DecodeBoxData_avc1(&(pBoxTree->mp4Box), &avc1Info);
    /* 数据引用索引 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDataRefIndex);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_DATA_REF_INDEX), pText, nId, hListView);
    /* 版本 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_VERSION), pText, nId, hListView);
    /* 修订 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nRevision);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_REVISION), pText, nId, hListView);
    /* 修订 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.cVendor);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_VENDOR), pText, nId, hListView);
    /* 时间质量 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nTemporalQuality);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_TEMPORAL_QUALITY), pText, nId, hListView);
    /* 空间质量 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nSpatialQuality);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_SPATIAL_QUALITY), pText, nId, hListView);
    /* 宽度（像素） */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nWidth);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_WIDTH), pText, nId, hListView);
    /* 高度（像素） */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nHeight);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_HEIGHT), pText, nId, hListView);
    /* 水平分辨率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g"), avc1Info.nHorizResolution.nPhy);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_HORIZ_RESOLUTION), pText, nId, hListView);
    /* 垂直分辨率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g"), avc1Info.nVertResolution.nPhy);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_VERT_RESOLUTION), pText, nId, hListView);
    /* 数据大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDataSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_DATA_SIZE), pText, nId, hListView);
    /* 每样本帧数 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nFrameCount);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_FRAME_COUNT), pText, nId, hListView);
    /* 压缩器名称 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, avc1Info.pCompressorName);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_COMPRESSOR_NAME), pText, nId, hListView);
    /* 颜色深度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDepth);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_DEPTH), pText, nId, hListView);
    /* 颜色表ID */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nColorTable);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_COLOR_TABLE), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_avcC(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_avcC avcCInfo;
    VIDEOH264PARAMETER_seq_parameter_set spsData;
    VIDEOH264PARAMETER_pic_parameter_set ppsData;
    uint32_t nLen;
    uint32_t nOffset;

    DecodeBoxData_avcC(&(pBoxTree->mp4Box), &avcCInfo);
    /* 配置版本 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nConfigurationVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_CONFIGURATION_VERSION), pText, nId, hListView);
    /* 配置文件标识 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), avcCInfo.nAvcProfileIndication, GetMP4_avcC_ProfileIndicationName(avcCInfo.nAvcProfileIndication));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_PROFILE_INDICATION), pText, nId, hListView);
    /* 配置文件兼容性 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nProfileCompatibility);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_PROFILE_COMPATIBILITY), pText, nId, hListView);
    /* 级别标识 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nAvcLevelIndication);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_LEVEL_INDICATION), pText, nId, hListView);
    /* NALU长度大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u字节"), avcCInfo.nNALULengthSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_NALU_LENGTH_SIZE), pText, nId, hListView);
    /* SPS 条目 */
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint8_t iLoop = 0; iLoop < avcCInfo.sps.nNum; iLoop++)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_SPS), iLoop + 1);
        nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nLen = ReadMP4Data2ByteMSB(&avcCInfo.sps.pList[nOffset]);
        if ((nLen + 2) <= avcCInfo.sps.nSize)
        {
            DecodeH264Parameter_seq_parameter_set(&(avcCInfo.sps.pList)[nOffset + 2], nLen, &spsData);
            nId = UpdateH264InfoWindow_SPS(&spsData, hListView, nId);
            nOffset += nLen + 2;
        }
        else { break; }
    }
    /* PPS 条目 */
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint8_t iLoop = 0; iLoop < avcCInfo.pps.nNum; iLoop++)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_PPS), iLoop + 1);
        nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nLen = ReadMP4Data2ByteMSB(&avcCInfo.pps.pList[nOffset]);
        if ((nLen + 2) <= avcCInfo.pps.nSize)
        {
            DecodeH264Parameter_pic_parameter_set(&(avcCInfo.pps.pList)[nOffset + 2], nLen, &ppsData, NULL);
            nId = UpdateH264InfoWindow_PPS(&ppsData, hListView, nId);
            nOffset += nLen + 2;
        }
        else { break; }
    }
}
static void UpdateBoxInfoWindow_pasp(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;

    /* 像素宽高比 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u:%u"), ReadMP4Data4ByteMSB(&pData[0]), ReadMP4Data4ByteMSB(&pData[4]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_pasp_FieldName(VIDEO_MP4_pasp_BOX_FIELD_PIXEL_ASPECT_RATIO), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_btrt(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;

    /* 解码缓存大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u 字节"), ReadMP4Data4ByteMSB(&pData[0]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_btrt_FieldName(VIDEO_MP4_btrt_BOX_FIELD_BUFFER_SIZE), pText, nId, hListView);
    /* 最大比特率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u bps"), ReadMP4Data4ByteMSB(&pData[4]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_btrt_FieldName(VIDEO_MP4_btrt_BOX_FIELD_MAX_BIT_RATE), pText, nId, hListView);
    /* 平均比特率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u bps"), ReadMP4Data4ByteMSB(&pData[8]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_btrt_FieldName(VIDEO_MP4_btrt_BOX_FIELD_AVG_BIT_RATE), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_stts(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsdSubBoxDefault sttsInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nSampleCount;
    uint32_t nSampleDelta;

    DecodeBoxData_stsdSubBoxDefault(&(pBoxTree->mp4Box), &sttsInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(sttsInfo.fbVF));
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < sttsInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nSampleCount = ReadMP4Data4ByteMSB(&(sttsInfo.pEntry)[nOffset]); nOffset += 4;
                nSampleDelta = ReadMP4Data4ByteMSB(&(sttsInfo.pEntry)[nOffset]); nOffset += 4;
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stts_FieldName(VIDEO_MP4_stts_BOX_FIELD_ENTRY), iLoop + 1);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSampleCount);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stts_FieldName(VIDEO_MP4_stts_BOX_FIELD_SAMPLE_COUNT), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSampleDelta);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stts_FieldName(VIDEO_MP4_stts_BOX_FIELD_SAMPLE_DELTA), pText, nId, hListView);
            }
            else
            {
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), sttsInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_ctts(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsdSubBoxDefault cttsInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nSampleCount;
    ULARGE_INTEGER nSampleOffset;

    DecodeBoxData_stsdSubBoxDefault(&(pBoxTree->mp4Box), &cttsInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(cttsInfo.fbVF));
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < cttsInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nSampleCount = ReadMP4Data4ByteMSB(&(cttsInfo.pEntry)[nOffset]); nOffset += 4;
                nOffset += DecodeBoxDataReadDataByVersion(&(cttsInfo.pEntry)[nOffset], cttsInfo.fbVF.nVersion, &nSampleOffset);
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_ctts_FieldName(VIDEO_MP4_ctts_BOX_FIELD_ENTRY), iLoop + 1);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSampleCount);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ctts_FieldName(VIDEO_MP4_ctts_BOX_FIELD_SAMPLE_COUNT), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), nSampleOffset.QuadPart);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ctts_FieldName(VIDEO_MP4_ctts_BOX_FIELD_SAMPLE_OFFSET), pText, nId, hListView);
            }
            else
            {
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), cttsInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stss(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsdSubBoxDefault stssInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;

    DecodeBoxData_stsdSubBoxDefault(&(pBoxTree->mp4Box), &stssInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stssInfo.fbVF));
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stssInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stss_FieldName(VIDEO_MP4_stss_BOX_FIELD_ENTRY), iLoop + 1);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("样本%u"), ReadMP4Data4ByteMSB(&(stssInfo.pEntry)[nOffset]));
                nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
                nOffset += 4;
            }
            else
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stssInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stsc(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsdSubBoxDefault stscInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nFirstChunk;
    uint32_t nSamplesPerChunk;
    uint32_t nSamplesDescriptionId;

    DecodeBoxData_stsdSubBoxDefault(&(pBoxTree->mp4Box), &stscInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stscInfo.fbVF));
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stscInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nFirstChunk = ReadMP4Data4ByteMSB(&(stscInfo.pEntry)[nOffset]); nOffset += 4;
                nSamplesPerChunk = ReadMP4Data4ByteMSB(&(stscInfo.pEntry)[nOffset]); nOffset += 4;
                nSamplesDescriptionId = ReadMP4Data4ByteMSB(&(stscInfo.pEntry)[nOffset]); nOffset += 4;
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_ENTRY), iLoop + 1);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nFirstChunk);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_FIRST_CHUNK), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSamplesPerChunk);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_SAMPLE_PER_CHUNK), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSamplesDescriptionId);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_SAMPLE_DESCRIPTION_ID), pText, nId, hListView);
            }
            else
            {
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stscInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stsz(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsz stszInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 12;
    HANSIZE nOffset;
    uint32_t nSize;

    DecodeBoxData_stsz(&(pBoxTree->mp4Box), &stszInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stszInfo.fbVF));
    if (0 != stszInfo.nSampleSize)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), stszInfo.nSampleSize);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsz_FieldName(VIDEO_MP4_stsz_BOX_FIELD_SAMPLE_SIZE), pText, nId, hListView);
    }
    else 
    {
        nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
        nOffset = 0;
        for (uint32_t iLoop = 0; iLoop < stszInfo.nSampleCnt; iLoop++)
        {
            if (nOffset < nLen)
            {
                if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
                {
                    nSize = ReadMP4Data4ByteMSB(&(stszInfo.pEntry)[nOffset]);
                    HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stsz_FieldName(VIDEO_MP4_stsz_BOX_FIELD_SIZE), iLoop + 1);
                    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSize);
                    nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
                }
                else
                {
                    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stszInfo.nSampleCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                    nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                    break;
                }
                nOffset += 4;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stco(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stco64 stco64Info;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nChunkOffset;

    DecodeBoxData_stco(&(pBoxTree->mp4Box), &stco64Info);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stco64Info.fbVF));
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stco64Info.nChunkCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nChunkOffset = ReadMP4Data4ByteMSB(&(stco64Info.pEntry)[nOffset]); nOffset += 4;
                HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stco_FieldName(VIDEO_MP4_stco_BOX_FIELD_ENTRY), iLoop + 1);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nChunkOffset);
                nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
            }
            else
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stco64Info.nChunkCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_co64(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stco64 stco64Info;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    ULARGE_INTEGER nChunkOffset;

    DecodeBoxData_co64(&(pBoxTree->mp4Box), &stco64Info);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stco64Info.fbVF));
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stco64Info.nChunkCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nChunkOffset.HighPart = ReadMP4Data4ByteMSB(&(stco64Info.pEntry)[nOffset]); nOffset += 4;
                nChunkOffset.LowPart = ReadMP4Data4ByteMSB(&(stco64Info.pEntry)[nOffset]); nOffset += 4;
                HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stco_FieldName(VIDEO_MP4_stco_BOX_FIELD_ENTRY), iLoop + 1);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), nChunkOffset.QuadPart);
                nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
            }
            else
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stco64Info.nChunkCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_smhd(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    int16_t nBalance;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 立体声平衡 */
    nBalance = ReadMP4Data2ByteMSB(&pData[4]);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1f"), (HANFLOAT)nBalance / (HANFLOAT)0x100);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_smhd_FieldName(VIDEO_MP4_smhd_BOX_FIELD_BALANCE), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_mp4a(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_mp4a mp4aInfo;

    DecodeBoxData_mp4a(&(pBoxTree->mp4Box), &mp4aInfo);
    /* ID */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_ID), pText, nId, hListView);
    /* 版本 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_VERSION), pText, nId, hListView);
    /* 修订 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nRevision);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_REVISION), pText, nId, hListView);
    /* 供应商 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nVendor);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_VENDOR), pText, nId, hListView);
    /* 声道数 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), mp4aInfo.nChannelCnt, GetMP4_mp4a_ChannelCountName(mp4aInfo.nChannelCnt));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_CHANNEL_COUNT), pText, nId, hListView);
    /* 样本大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nSampleSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_SAMPLE_SIZE), pText, nId, hListView);
    /* 压缩ID */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nCompressionId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_COMPRESSION_ID), pText, nId, hListView);
    /* 数据包大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nPacketSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_PACKET_SIZE), pText, nId, hListView);
    /* 采样率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g Hz"), mp4aInfo.nSampleRate);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_SAMPLE_RATE), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_meta(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    nId = UpdateBoxInfoWindow_BlankBox(&pData[4], pBoxTree->mp4Box.nDataLen - 4, nId, hListView);
}
static void UpdateBoxInfoWindow_data(PVIDEOMP4BOXTREE pBoxTree, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HWND hListView = mp4Info->box.hInfo;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    WCHAR pWText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    CHAR pMText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANINT nId = 0;
    uint32_t cDataType = ReadMP4Data4ByteMSB(&pData[0]);
    uint32_t cLocale = ReadMP4Data4ByteMSB(&pData[4]);
    const uint8_t* pDataData = &pData[8];
    HANSIZE nDataLen = pBoxTree->mp4Box.nDataLen - 8;
    HANINT nTextSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE;
    HANINT nWTextSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE;

    /* 数据类型 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), cDataType, GetMP4_data_DataTypeName(cDataType));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_data_FieldName(VIDEO_MP4_data_BOX_DATA_TYPE), pText, nId, hListView);
    /* 语言/地区代码 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), cLocale);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_data_FieldName(VIDEO_MP4_data_BOX_LOCALE), pText, nId, hListView);
    /* 数据 */
    switch (cDataType) {
        case 1: {
            if (HAN_VIDEO_MP4_TEXT_BUF_SIZE <= nDataLen) { nDataLen = HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1; }
            memcpy(pMText, pDataData, nDataLen);
            pMText[nDataLen] = '\0';
            MultiByteToHANStr(CP_UTF8, 0, (PCCH)pDataData, -1, pText, &nTextSize, pWText, &nWTextSize, NULL, NULL);
        } break;
        default: {
            VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pDataData, nDataLen);
        } break;
    }
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_data_FieldName(VIDEO_MP4_data_BOX_DATA), pText, nId, hListView);
}

static void UpdateBoxInfoWindow_mdatSample(HANSIZE nTrackId, HANSIZE nSampleId, PVIDEOMP4WNDEXTRA mp4Info)
{
    const uint8_t* pData = mp4Info->paramVideo.pData;
    HWND hListView = mp4Info->box.hInfo;
    PVIDEOMP4BOXINFO pBoxInfo = &(mp4Info->box.boxInfo);
    PVIDEOMP4TRACK pTrack = pBoxInfo->track.pList;
    PVIDEOMP4DECODEINFOSAMPLE pSample = &(pTrack[nTrackId].pSample[nSampleId]);
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    const uint8_t* pSampleData;
    VIDEOH264PARAMETER_slice_layer_without_partitioning sliceInfo;
    
    pSampleData = &pData[pTrack[nTrackId].pSample[nSampleId].sOffset];
    sliceInfo.SPS.nCnt = pTrack[nTrackId].avcC.sps.nNum;
    sliceInfo.SPS.pSPS = pTrack[nTrackId].avcC.sps.pSPS;
    sliceInfo.PPS.nCnt = pTrack[nTrackId].avcC.pps.nNum;
    sliceInfo.PPS.pPPS = pTrack[nTrackId].avcC.pps.pPPS;
    DecodeH264Parameter_slice_layer_without_partitioning(&pSampleData[4], ReadMP4Data4ByteMSB(pSampleData), &sliceInfo);
    
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nTrackId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_TRACK_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSample->idChunkGroup);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_CHUNK_GROUP_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSample->idChunk);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_CHUNK_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nSampleId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_SAMPLE_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->sOffset);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_OFFSET), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u 字节"), pSample->nSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_SAMPLE_SIZE), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->timeDTS);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_DTS), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->timePTS);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_PTS), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->timeDuration);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_DURATION), pText, nId, hListView);

    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nId = UpdateH264InfoWindow_Slice(&sliceInfo, hListView, nId);
}

static HANINT UpdateH264InfoWindow_SPS(PCVIDEOH264PARAMETER_seq_parameter_set pSPS, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->nal_ref_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_ref_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->nal_unit_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_unit_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->profile_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("profile_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set0_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set0_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set1_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set1_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set2_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set2_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set3_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set3_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set4_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set4_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set5_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set5_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->level_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("level_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->seq_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("seq_parameter_set_id"), pText, nRet, hListView);
    if (TRUE == SPSProfileIdcHasExtendedData(pSPS->profile_idc))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->chroma_format_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_format_idc"), pText, nRet, hListView);
        if (3 == pSPS->chroma_format_idc)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->separate_colour_plane_flag);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("separate_colour_plane_flag"), pText, nRet, hListView);
        }
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->bit_depth_luma_minus8);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bit_depth_luma_minus8"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->bit_depth_chroma_minus8);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bit_depth_chroma_minus8"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->qpprime_y_zero_transform_bypass_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("qpprime_y_zero_transform_bypass_flag"), pText, nRet, hListView);
        if (0 != pSPS->seq_scaling_matrix_present_flag)
        {
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("scaling_list 未实现"), TEXT(""), nRet, hListView);
        }
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->log2_max_frame_num_minus4);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_frame_num_minus4"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->pic_order_cnt_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_order_cnt_type"), pText, nRet, hListView);
    if (0 == pSPS->pic_order_cnt_type)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->log2_max_pic_order_cnt_lsb_minus4);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_pic_order_cnt_lsb_minus4"), pText, nRet, hListView);
    }
    else if (1 == pSPS->pic_order_cnt_type)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->delta_pic_order_always_zero_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_always_zero_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSPS->offset_for_non_ref_pic);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("offset_for_non_ref_pic"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSPS->offset_for_top_to_bottom_field);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("offset_for_top_to_bottom_field"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->num_ref_frames_in_pic_order_cnt_cycle);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_frames_in_pic_order_cnt_cycle"), pText, nRet, hListView);
    }
    else { }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->max_num_ref_frames);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_num_ref_frames"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->gaps_in_frame_num_value_allowed_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("gaps_in_frame_num_value_allowed_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->pic_width_in_mbs_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_width_in_mbs_minus1"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->pic_height_in_map_units_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_height_in_map_units_minus1"), pText, nRet, hListView);
    if (0 == pSPS->frame_mbs_only_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->mb_adaptive_frame_field_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("mb_adaptive_frame_field_flag"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->direct_8x8_inference_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("direct_8x8_inference_flag"), pText, nRet, hListView);
    if (0 != pSPS->frame_cropping_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_left_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_left_offset"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_right_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_right_offset"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_top_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_top_offset"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_bottom_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_bottom_offset"), pText, nRet, hListView);
    }
    if (0 != pSPS->vui_parameters_present_flag)
    {
        nRet = UpdateH264InfoWindow_VUIParam(&(pSPS->vui_parameters), hListView, nRet);
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_VUIParam(PCVIDEOH264PARAMETER_vui_parameters pVUI, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if (0 != pVUI->aspect_ratio_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->aspect_ratio_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("aspect_ratio_idc"), pText, nRet, hListView);
        if (VIDEO_H264_EXTENDED_SAR == pVUI->aspect_ratio_idc)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u:%u"), pVUI->sar_width, pVUI->sar_height);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("Extended_SAR"), pText, nRet, hListView);
        }
    }
    if (0 != pVUI->overscan_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->overscan_appropriate_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("overscan_appropriate_flag"), pText, nRet, hListView);
    }
    if (0 != pVUI->video_signal_type_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->video_format);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("video_format"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->video_full_range_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("video_full_range_flag"), pText, nRet, hListView);
        if (0 != pVUI->colour_description_present_flag)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->colour_primaries);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("colour_primaries"), pText, nRet, hListView);
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->transfer_characteristics);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("transfer_characteristics"), pText, nRet, hListView);
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->matrix_coefficients);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("matrix_coefficients"), pText, nRet, hListView);
        }
    }
    if (0 != pVUI->chroma_loc_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->chroma_sample_loc_type_top_field);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_sample_loc_type_top_field"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->chroma_sample_loc_type_bottom_field);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_sample_loc_type_bottom_field"), pText, nRet, hListView);
    }
    if (0 != pVUI->timing_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->num_units_in_tick);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_units_in_tick"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->time_scale);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("time_scale"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->fixed_frame_rate_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("fixed_frame_rate_flag"), pText, nRet, hListView);
    }
    if (0 != pVUI->nal_hrd_parameters_present_flag) { nRet = UpdateH264InfoWindow_HRDParam(&(pVUI->nal.hrd_parameters), TEXT("nal."), hListView, nRet); }
    if (0 != pVUI->vcl_hrd_parameters_present_flag) { nRet = UpdateH264InfoWindow_HRDParam(&(pVUI->vcl.hrd_parameters), TEXT("vcl."), hListView, nRet); }
    if ((0 != pVUI->nal_hrd_parameters_present_flag) || (0 != pVUI->vcl_hrd_parameters_present_flag))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->low_delay_hrd_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("low_delay_hrd_flag"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->pic_struct_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_struct_present_flag"), pText, nRet, hListView);
    if (0 != pVUI->bitstream_restriction_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->motion_vectors_over_pic_boundaries_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("motion_vectors_over_pic_boundaries_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_bytes_per_pic_denom);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_bytes_per_pic_denom"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_bits_per_mb_denom);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_bits_per_mb_denom"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->log2_max_mv_length_horizontal);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_mv_length_horizontal"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->log2_max_mv_length_vertical);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_mv_length_vertical"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_num_reorder_frames);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_num_reorder_frames"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_dec_frame_buffering);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_dec_frame_buffering"), pText, nRet, hListView);
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_HRDParam(PCVIDEOH264PARAMETER_hrd_parameters pHRD, HANPCSTR pHeader, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANPSTR pFieldName;
    HANSIZE nTextLen;
    HANSIZE nFieldSize;

    HAN_strncpy(pField, pHeader, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
    pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1] = TEXT('\0');
    nTextLen = HAN_strlen(pField);
    pFieldName = &pField[nTextLen];
    nFieldSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE - nTextLen;

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->cpb_cnt_minus1);
    HAN_strncpy(pFieldName, TEXT("cpb_cnt_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->bit_rate_scale);
    HAN_strncpy(pFieldName, TEXT("bit_rate_scale"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->cpb_size_scale);
    HAN_strncpy(pFieldName, TEXT("cpb_size_scale"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->initial_cpb_removal_delay_length_minus1);
    HAN_strncpy(pFieldName, TEXT("initial_cpb_removal_delay_length_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->cpb_removal_delay_length_minus1);
    HAN_strncpy(pFieldName, TEXT("cpb_removal_delay_length_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->dpb_output_delay_length_minus1);
    HAN_strncpy(pFieldName, TEXT("dpb_output_delay_length_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->time_offset_length);
    HAN_strncpy(pFieldName, TEXT("time_offset_length"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);

    return nRet;
}

static HANINT UpdateH264InfoWindow_PPS(PCVIDEOH264PARAMETER_pic_parameter_set pPPS, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->nal_ref_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_ref_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->nal_unit_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_unit_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->pic_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_parameter_set_id"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->seq_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("seq_parameter_set_id"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->entropy_coding_mode_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("entropy_coding_mode_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->bottom_field_pic_order_in_frame_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bottom_field_pic_order_in_frame_present_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->num_slice_groups_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_slice_groups_minus1"), pText, nRet, hListView);
    if (0 < pPPS->num_slice_groups_minus1)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->slice_group_map_type);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_group_map_type"), pText, nRet, hListView);
        if (0 == pPPS->slice_group_map_type)
        {
            ;
        }
        else if (2 == pPPS->slice_group_map_type)
        {
            ;
        }
        else if ((3 == pPPS->slice_group_map_type) ||
                 (4 == pPPS->slice_group_map_type) ||
                 (5 == pPPS->slice_group_map_type))
        {
            ;
        }
        else if (6 == pPPS->slice_group_map_type)
        {
            ;
        }
        else { }
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->num_ref_idx_l0_default_active_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l0_default_active_minus1"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->num_ref_idx_l1_default_active_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l1_default_active_minus1"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->weighted_pred_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("weighted_pred_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->weighted_bipred_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("weighted_bipred_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pPPS->pic_init_qp_minus26);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_init_qp_minus26"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pPPS->pic_init_qs_minus26);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_init_qs_minus26"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pPPS->chroma_qp_index_offset);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_qp_index_offset"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->deblocking_filter_control_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("deblocking_filter_control_present_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->constrained_intra_pred_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constrained_intra_pred_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->redundant_pic_cnt_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("redundant_pic_cnt_present_flag"), pText, nRet, hListView);
    if (TRUE == pPPS->bMoreRbspData)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->transform_8x8_mode_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("transform_8x8_mode_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->pic_scaling_matrix_present_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_scaling_matrix_present_flag"), pText, nRet, hListView);
        if (0 != pPPS->pic_scaling_matrix_present_flag)
        {
            ;
        }
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->second_chroma_qp_index_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("second_chroma_qp_index_offset"), pText, nRet, hListView);
    }

    return nRet;
}

static HANINT UpdateH264InfoWindow_Slice(PCVIDEOH264PARAMETER_slice_layer_without_partitioning pSlice, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    PCVIDEOH264PARAMETER_seq_parameter_set pSPS = pSlice->SPS.pSPS;
    PCVIDEOH264PARAMETER_pic_parameter_set pPPS = pSlice->PPS.pPPS;
    uint8_t idrPicFlag = GetIdrPicFlag(pSlice->slice_header.nal_unit_type);
    VIDEOH264SLICETYPE sliceType = pSlice->slice_header.slice_type % VIDEO_H264_SLICE_TYPE_ALL;

    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("Slice Header"), TEXT(""), nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.nal_ref_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_ref_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.nal_unit_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_unit_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.first_mb_in_slice);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("first_mb_in_slice"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.slice_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.pic_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_parameter_set_id"), pText, nRet, hListView);
    if (1 == pSPS->separate_colour_plane_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.colour_plane_id);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("colour_plane_id"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.frame_num);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_num"), pText, nRet, hListView);
    if (0 == pSPS->frame_mbs_only_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.field_pic_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("field_pic_flag"), pText, nRet, hListView);
        if (0 != pSlice->slice_header.field_pic_flag)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.bottom_field_flag);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bottom_field_flag"), pText, nRet, hListView);
        }
    }
    if (0 != idrPicFlag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.idr_pic_id);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("idr_pic_id"), pText, nRet, hListView);
    }
    if (0 == pSPS->pic_order_cnt_type)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.pic_order_cnt_lsb);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_order_cnt_lsb"), pText, nRet, hListView);
        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pSlice->slice_header.field_pic_flag))
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.delta_pic_order_cnt_bottom);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_cnt_bottom"), pText, nRet, hListView);
        }
    }
    if ((1 == pSPS->pic_order_cnt_type) && (0 == pSPS->delta_pic_order_always_zero_flag))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.delta_pic_order_cnt[0]);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_cnt[0]"), pText, nRet, hListView);
        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pSlice->slice_header.field_pic_flag))
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.delta_pic_order_cnt[1]);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_cnt[1]"), pText, nRet, hListView);
        }
    }
    if (0 != pPPS->redundant_pic_cnt_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.redundant_pic_cnt);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("redundant_pic_cnt"), pText, nRet, hListView);
    }
    if (VIDEO_H264_SLICE_TYPE_B == sliceType)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.direct_spatial_mv_pred_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("direct_spatial_mv_pred_flag"), pText, nRet, hListView);
    }
    if ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_B == sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.num_ref_idx_active_override_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_active_override_flag"), pText, nRet, hListView);
        if (0 != pSlice->slice_header.num_ref_idx_active_override_flag)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.num_ref_idx_l0_active_minus1);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l0_active_minus1"), pText, nRet, hListView);
            if (VIDEO_H264_SLICE_TYPE_B == sliceType)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.num_ref_idx_l1_active_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l1_active_minus1"), pText, nRet, hListView);
            }
        }
    }
    if ((20 == pSlice->slice_header.nal_unit_type) || (21 == pSlice->slice_header.nal_unit_type))
    {
        nRet = UpdateH264InfoWindow_RefPicListMVCModification(&(pSlice->slice_header.ref_pic_list_mvc), sliceType, hListView, nRet);
    }
    else
    {
        nRet = UpdateH264InfoWindow_RefPicListModification(&(pSlice->slice_header.ref_pic_list_mvc), sliceType, hListView, nRet);
    }
    if (((0 != pPPS->weighted_pred_flag) && ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType))) ||
        ((1 == pPPS->weighted_bipred_idc) && (VIDEO_H264_SLICE_TYPE_B == sliceType)))
    {
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pred_weight_table"), TEXT("未实现"), nRet, hListView);
    }
    if (0 != pSlice->slice_header.nal_ref_idc)
    {
        nRet = UpdateH264InfoWindow_DecRefPicMarking(&(pSlice->slice_header.dec_ref_pic_marking), idrPicFlag, hListView, nRet);
    }
    if ((0 != pPPS->entropy_coding_mode_flag) && (VIDEO_H264_SLICE_TYPE_I != sliceType) && (VIDEO_H264_SLICE_TYPE_SI != sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.cabac_init_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("cabac_init_idc"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_qp_delta);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_qp_delta"), pText, nRet, hListView);
    if ((VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_SI == sliceType))
    {
        if (VIDEO_H264_SLICE_TYPE_SP == sliceType)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.sp_for_switch_flag);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("sp_for_switch_flag"), pText, nRet, hListView);
        }
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_qs_delta);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_qs_delta"), pText, nRet, hListView);
    }
    if (0 != pPPS->deblocking_filter_control_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.disable_deblocking_filter_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("disable_deblocking_filter_idc"), pText, nRet, hListView);
        if (1 != pSlice->slice_header.disable_deblocking_filter_idc)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_alpha_c0_offset_div2);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_alpha_c0_offset_div2"), pText, nRet, hListView);
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_beta_offset_div2);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_beta_offset_div2"), pText, nRet, hListView);
        }
    }
    if ((0 < pPPS->num_slice_groups_minus1) && (3 <= pPPS->slice_group_map_type) && (pPPS->slice_group_map_type <= 5))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.slice_group_change_cycle);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_group_change_cycle"), pText, nRet, hListView);
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_RefPicListMVCModification(PCVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if ((2 != sliceType) && (4 != sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->ref_pic_list_modification_flag_l0);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l0"), pText, nRet, hListView);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l0)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicListMVC->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_abs_diff_view_idx_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_view_idx_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_view_idx_minus1"), pText, nRet, hListView);
            }
        }
    }
    if (1 == sliceType)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->ref_pic_list_modification_flag_l1);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l1"), pText, nRet, hListView);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l1)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicListMVC->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_abs_diff_view_idx_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_view_idx_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_view_idx_minus1"), pText, nRet, hListView);
            }
        }
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_RefPicListModification(PCVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if ((2 != sliceType) && (4 != sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->ref_pic_list_modification_flag_l0);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l0"), pText, nRet, hListView);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l0)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicList->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicList->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
        }
    }
    if (1 == sliceType)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->ref_pic_list_modification_flag_l1);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l1"), pText, nRet, hListView);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l1)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicList->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicList->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
        }
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_DecRefPicMarking(PCVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if (0 != idrPicFlag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->no_output_of_prior_pics_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("no_output_of_prior_pics_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->long_term_reference_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_reference_flag"), pText, nRet, hListView);
    }
    else
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("adaptive_ref_pic_marking_mode_flag"), pText, nRet, hListView);
        if (0 != pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag)
        {
            if (0 != pDecRefPicMarking->bValid_difference_of_pic_nums_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->difference_of_pic_nums_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("difference_of_pic_nums_minus1"), pText, nRet, hListView);
            }
            if (0 != pDecRefPicMarking->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
            if (0 != pDecRefPicMarking->bValid_long_term_frame_idx)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->long_term_frame_idx);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_frame_idx"), pText, nRet, hListView);
            }
            if (0 != pDecRefPicMarking->bValid_max_long_term_frame_idx_plus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->max_long_term_frame_idx_plus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_long_term_frame_idx_plus1"), pText, nRet, hListView);
            }
        }
    }

    return nRet;
}

static HANSIZE DecodeBoxDataReadDataByVersion(const uint8_t* pData, uint8_t nVersion, PULARGE_INTEGER pLargeInt)
{
    HANSIZE nRet;
    
    switch (nVersion) {
        case 0: {
            pLargeInt->HighPart = 0;
            pLargeInt->LowPart = ReadMP4Data4ByteMSB(&pData[0]);
            nRet = 4;
        } break;
        case 1: {
            pLargeInt->HighPart = ReadMP4Data4ByteMSB(&pData[0]);
            pLargeInt->LowPart = ReadMP4Data4ByteMSB(&pData[4]);
            nRet = 8;
        } break;
        default: { nRet = 0; } break;
    }

    return nRet;
}
static HANSIZE DecodeBoxDataDateTime(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFODATETIME pDateTime)
{
    HANSIZE nRet = DecodeBoxDataReadDataByVersion(pData, nVersion, &(pDateTime->uTime));
    ULARGE_INTEGER uTime;
    FILETIME ftStd;
    FILETIME ftLocal;

    uTime.QuadPart = (pDateTime->uTime.QuadPart + 9561628800U) * 10000000U;
    ftStd.dwLowDateTime = uTime.LowPart;
    ftStd.dwHighDateTime = uTime.HighPart;
    FileTimeToLocalFileTime(&ftStd, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &(pDateTime->stTime));

    return nRet;
}
static void DecodeBoxData_FullBoxVersionFlags(const uint8_t* pData, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB)
{
    pFB->nVersion = pData[0];
    pFB->cFlags = ReadMP4Data3ByteMSB(&pData[1]);
}
static BOOL DecodeBoxData_mvhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mvhd pmvhd)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    pmvhd->pRawData = pData;
    pmvhd->fbVF.nVersion = pData[nOffset]; nOffset += 1;
    if (pmvhd->fbVF.nVersion < 2)
    {
        pmvhd->fbVF.cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmvhd->fbVF.nVersion, &(pmvhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmvhd->fbVF.nVersion, &(pmvhd->modificationTime));
        pmvhd->timeDuration.nTimescale = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], pmvhd->fbVF.nVersion, &(pmvhd->timeDuration.nDuration));
        pmvhd->timeDuration.sTimeDuration.QuadPart = pmvhd->timeDuration.nDuration.QuadPart / pmvhd->timeDuration.nTimescale;
        pmvhd->nRate.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        pmvhd->nRate.nPhy += (HANDOUBLE)(pmvhd->nRate.u32) / (HANDOUBLE)0x10000;
        pmvhd->nVolume.u16 = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        pmvhd->nVolume.nPhy = (HANDOUBLE)(pmvhd->nVolume.u16) / (HANDOUBLE)0x100;
        nOffset += 10;
        memcpy(pmvhd->pMatrix, &pData[nOffset], sizeof(pmvhd->pMatrix)); nOffset += sizeof(pmvhd->pMatrix);
        nOffset += 24;
        pmvhd->nNextTrackId = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    }
    else { bRet = FALSE; }

    return bRet;
}
static BOOL DecodeBoxData_tkhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_tkhd ptkhd)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset;
    
    ptkhd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(ptkhd->fbVF));
    if (ptkhd->fbVF.nVersion < 2)
    {
        nOffset = 4;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], ptkhd->fbVF.nVersion, &(ptkhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], ptkhd->fbVF.nVersion, &(ptkhd->modificationTime));
        ptkhd->nTrackId = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], ptkhd->fbVF.nVersion, &(ptkhd->nDuration));
        nOffset += 8;
        ptkhd->nLayer = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        ptkhd->nAlternateGroup = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        ptkhd->nVolume.u16 = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        ptkhd->nVolume.nPhy = (HANDOUBLE)(ptkhd->nVolume.u16) / (HANDOUBLE)0x100;
        nOffset += 2;
        memcpy(ptkhd->pMatrix, &pData[nOffset], sizeof(ptkhd->pMatrix)); nOffset += sizeof(ptkhd->pMatrix);
        ptkhd->nWidth.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        ptkhd->nWidth.nPhy = (HANDOUBLE)(ptkhd->nWidth.u32) / (HANDOUBLE)0x10000;
        ptkhd->nHeight.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        ptkhd->nHeight.nPhy = (HANDOUBLE)(ptkhd->nHeight.u32) / (HANDOUBLE)0x10000;
    }
    else { bRet = FALSE; }

    return bRet;
}
static HANSIZE DecodeBoxData_elst(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFO_elst pelst)
{
    HANSIZE nRet = 0;

    pelst->pRawData = pData;
    if (nVersion < 2)
    {
        nRet += DecodeBoxDataReadDataByVersion(&pData[nRet], nVersion, &(pelst->nSegmentDuration));
        nRet += DecodeBoxDataReadDataByVersion(&pData[nRet], nVersion, &(pelst->nMediaTime));
        pelst->mediaRate.u32 = ReadMP4Data4ByteMSB(&pData[nRet]); nRet += 4;
        pelst->mediaRate.nPhy = (HANDOUBLE)(pelst->mediaRate.u32) / (HANDOUBLE)0x10000;
    }

    return nRet;
}
static BOOL DecodeBoxData_mdhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mdhd pmdhd)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset;
    
    pmdhd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pmdhd->fbVF));
    if (pmdhd->fbVF.nVersion < 2)
    {
        nOffset = 4;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmdhd->fbVF.nVersion, &(pmdhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmdhd->fbVF.nVersion, &(pmdhd->modificationTime));
        pmdhd->timeDuration.nTimescale = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], pmdhd->fbVF.nVersion, &(pmdhd->timeDuration.nDuration));
        pmdhd->timeDuration.sTimeDuration.QuadPart = pmdhd->timeDuration.nDuration.QuadPart / pmdhd->timeDuration.nTimescale;
        memcpy(pmdhd->pLanguage, &pData[nOffset], 2); nOffset += 2;
    }
    else { bRet = FALSE; }

    return bRet;
}
static void DecodeBoxData_hdlr(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_hdlr phdlr)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 8;
    
    phdlr->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(phdlr->fbVF));
    memcpy(phdlr->handlerType.pType, &pData[nOffset], 4); nOffset += 4;
    phdlr->handlerType.pType[4] = '\0';
    phdlr->handlerType.eType = DecodeBoxDataGetTrackHandlerType(phdlr->handlerType.pType);
    nOffset += 12;
    phdlr->pName = &pData[nOffset];
}
static void DecodeBoxData_vmhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_vmhd pvmhd)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 4;
    
    pvmhd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pvmhd->fbVF));
    pvmhd->cGraphicsMode = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pvmhd->pOpColor[0] = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pvmhd->pOpColor[1] = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pvmhd->pOpColor[2] = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
}
static VIDEOMP4TRACKHANDLERTYPE DecodeBoxDataGetTrackHandlerType(uint8_t pType[4])
{
    VIDEOMP4TRACKHANDLERTYPE eRet = VIDEO_MP4_TRACK_HANDLER_TYPE_CNT;

    for (VIDEOMP4TRACKHANDLERTYPE iLoop = 0; iLoop < VIDEO_MP4_TRACK_HANDLER_TYPE_CNT; iLoop++)
    {
        if (!memcmp(pType, sg_pMP4BoxTrackHandlerInfo[iLoop].pType, 4))
        {
            eRet = iLoop;
            break;
        }
    }

    return eRet;
}
static BOOL DecodeBoxData_url_(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_url_ purl_)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    purl_->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(purl_->fbVF));
    switch (purl_->fbVF.cFlags) {
        case 0x000000: { purl_->pUrl = &pData[nOffset]; } break;
        case 0x000001: { purl_->pUrl = NULL; } break;
        default: { bRet = FALSE; } break;
    }

    return bRet;
}
static void DecodeBoxData_avc1(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avc1 pavc1)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 0;
    
    nOffset += 6;
    pavc1->pRawData = pData;
    pavc1->nDataRefIndex = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nVersion = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nRevision = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->cVendor = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nTemporalQuality = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nSpatialQuality = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nWidth = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nHeight = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nHorizResolution.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nHorizResolution.nPhy = (HANDOUBLE)(pavc1->nHorizResolution.u32) / (HANDOUBLE)0x10000;
    pavc1->nVertResolution.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nVertResolution.nPhy = (HANDOUBLE)(pavc1->nVertResolution.u32) / (HANDOUBLE)0x10000;
    pavc1->nDataSize = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nFrameCount = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    memcpy(pavc1->pCompressorName, &pData[nOffset], 32); nOffset += 32;
    pavc1->nDepth = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nColorTable = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
}
static void DecodeBoxData_avcC(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avcC pavcC)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 0;
    uint16_t nUnitLen;
    
    pavcC->pRawData = pData;
    pavcC->nConfigurationVersion = pData[nOffset]; nOffset += 1;
    pavcC->nAvcProfileIndication = pData[nOffset]; nOffset += 1;
    pavcC->nProfileCompatibility = pData[nOffset]; nOffset += 1;
    pavcC->nAvcLevelIndication = pData[nOffset]; nOffset += 1;
    pavcC->nNALULengthSize = (pData[nOffset] & 0x03) + 1; nOffset += 1;
    pavcC->sps.nNum = (pData[nOffset] & 0x1F); nOffset += 1;
    pavcC->sps.pList = &pData[nOffset];
    pavcC->sps.nSize = 0;
    for (uint8_t iLoop = 0; iLoop < pavcC->sps.nNum; iLoop++)
    {
        nUnitLen = ReadMP4Data2ByteMSB(&pData[nOffset]);
        nOffset += (HANSIZE)2 + nUnitLen;
        pavcC->sps.nSize += (HANSIZE)2 + nUnitLen;
    }
    pavcC->pps.nNum = pData[nOffset]; nOffset += 1;
    pavcC->pps.pList = &pData[nOffset];
    for (uint8_t iLoop = 0; iLoop < pavcC->pps.nNum; iLoop++)
    {
        nUnitLen = ReadMP4Data2ByteMSB(&pData[nOffset]);
        nOffset += (HANSIZE)2 + nUnitLen;
        pavcC->pps.nSize += (HANSIZE)2 + nUnitLen;
    }
}
static void DecodeBoxData_stsdSubBoxDefault(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsdSubBoxDefault pDef)
{
    const uint8_t* pData = pBox->pData;

    pDef->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pDef->fbVF));
    pDef->nCnt = ReadMP4Data4ByteMSB(&pData[4]);
    pDef->pEntry = &pData[8];
    pDef->nEntrySize = pBox->nDataLen - 8;
}
static void DecodeBoxData_stsz(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsz pstsz)
{
    const uint8_t* pData = pBox->pData;

    pstsz->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pstsz->fbVF));
    pstsz->nSampleSize = ReadMP4Data4ByteMSB(&pData[4]);
    pstsz->nSampleCnt = ReadMP4Data4ByteMSB(&pData[8]);
    if (0 == pstsz->nSampleSize)
    {
        pstsz->pEntry = &pData[12];
        pstsz->nEntrySize = pBox->nDataLen - 12;
    }
    else
    {
        pstsz->pEntry = NULL;
        pstsz->nEntrySize = 0;
    }
}
static void DecodeBoxData_stco(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pstco)
{
    const uint8_t* pData = pBox->pData;

    pstco->pRawData = pData;
    pstco->nDataSize = 4;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pstco->fbVF));
    pstco->nChunkCnt = ReadMP4Data4ByteMSB(&pData[4]);
    pstco->pEntry = &pData[8];
    pstco->nEntrySize = pBox->nDataLen - 8;
}
static void DecodeBoxData_co64(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pco64)
{
    const uint8_t* pData = pBox->pData;

    pco64->pRawData = pData;
    pco64->nDataSize = 8;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pco64->fbVF));
    pco64->nChunkCnt = ReadMP4Data4ByteMSB(&pData[4]);
    pco64->pEntry = &pData[8];
    pco64->nEntrySize = pBox->nDataLen - 8;
}
static void DecodeBoxData_mp4a(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mp4a pmp4a)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 6;
    uint32_t nSampleRate;
    
    pmp4a->pRawData = pData;
    pmp4a->nId = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nVersion = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nRevision = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nVendor = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pmp4a->nChannelCnt = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nSampleSize = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nCompressionId = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nPacketSize = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    nSampleRate = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pmp4a->nSampleRate = (HANDOUBLE)nSampleRate / (HANDOUBLE)0x10000;
}

#if 1 /* 解码 */
static BOOL DecodeMP4(PVIDEOMP4WNDEXTRA mp4Info, HANSIZE nFrame)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = mp4Info->paramVideo.pData;
    PVIDEOMP4BOXINFO pBoxInfo = &(mp4Info->box.boxInfo);
    PVIDEOMP4TRACK pTrack = pBoxInfo->track.pList;
    HANSIZE nTrackCnt = pBoxInfo->track.nCnt;
    const uint8_t* pSampleData;
    uint32_t nSampleDataSize;

    VIDEOH264PARAMETER_slice_layer_without_partitioning sliceInfo;

    for (HANSIZE iLoop = 0; iLoop < nTrackCnt; iLoop++)
    {
        if (VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO == pTrack[iLoop].hdlr.handlerType.eType)
        {
            pSampleData = &pData[pTrack[iLoop].pSample[nFrame].sOffset];
            nSampleDataSize = ReadMP4Data4ByteMSB(pSampleData);
            sliceInfo.SPS.nCnt = pTrack[iLoop].avcC.sps.nNum;
            sliceInfo.SPS.pSPS = pTrack[iLoop].avcC.sps.pSPS;
            sliceInfo.PPS.nCnt = pTrack[iLoop].avcC.pps.nNum;
            sliceInfo.PPS.pPPS = pTrack[iLoop].avcC.pps.pPPS;
            DecodeH264Parameter_slice_layer_without_partitioning(&pSampleData[4], nSampleDataSize, &sliceInfo);
            break;
        }
    }

    return bRet;
}
#endif
