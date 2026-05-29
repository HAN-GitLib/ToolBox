#include "HAN_PictureJpegMakerNoteSonyModelInfo.h"
#include "HAN_PictureJpegMakerNoteSonyTag.h"

typedef struct tagPICTUREJPEGMAKERNOTESONYMODELINFO {
    PICTUREJPEGMAKERNOTESONYMODELID     nId;
    const CHAR*                         pModelName;
    void                                (*ReadCameraInfo)(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
} PICTUREJPEGMAKERNOTESONYMODELINFO;

static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3(PPICTUREJPEGSEGMENTREADEXIF pReadExif);

static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(HANPSTR pText, size_t nTextSize, int16_t nStatus);

PICTUREJPEGMAKERNOTESONYMODELINFO sg_pSonyModelInfo[PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_CNT] = {
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_R1] = {
        .nId = 2,
        .pModelName = "DSC-R1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A100] = {
        .nId = 256,
        .pModelName = "DSLR-A100",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A900] = {
        .nId = 257,
        .pModelName = "DSLR-A900",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A700] = {
        .nId = 258,
        .pModelName = "DSLR-A700",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A200] = {
        .nId = 259,
        .pModelName = "DSLR-A200",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A350] = {
        .nId = 260,
        .pModelName = "DSLR-A350",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A300] = {
        .nId = 261,
        .pModelName = "DSLR-A300",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A900_APS_Cmode] = {
        .nId = 262,
        .pModelName = "DSLR-A900(APS-Cmode)",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A380] = {
        .nId = 263,
        .pModelName = "DSLR-A380",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A390] = {
        .nId = 263,
        .pModelName = "DSLR-A390",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A330] = {
        .nId = 264,
        .pModelName = "DSLR-A330",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A230] = {
        .nId = 265,
        .pModelName = "DSLR-A230",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A290] = {
        .nId = 266,
        .pModelName = "DSLR-A290",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A850] = {
        .nId = 269,
        .pModelName = "DSLR-A850",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A850_APS_Cmode] = {
        .nId = 270,
        .pModelName = "DSLR-A850(APS-Cmode)",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A550] = {
        .nId = 273,
        .pModelName = "DSLR-A550",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A500] = {
        .nId = 274,
        .pModelName = "DSLR-A500",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A450] = {
        .nId = 275,
        .pModelName = "DSLR-A450",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_5] = {
        .nId = 278,
        .pModelName = "NEX-5",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_3] = {
        .nId = 279,
        .pModelName = "NEX-3",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A33] = {
        .nId = 280,
        .pModelName = "SLT-A33",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A55] = {
        .nId = 281,
        .pModelName = "SLT-A55",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A55V] = {
        .nId = 281,
        .pModelName = "SLT-A55V",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A560] = {
        .nId = 282,
        .pModelName = "DSLR-A560",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSLR_A580] = {
        .nId = 283,
        .pModelName = "DSLR-A580",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_C3] = {
        .nId = 284,
        .pModelName = "NEX-C3",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A35] = {
        .nId = 285,
        .pModelName = "SLT-A35",
        .ReadCameraInfo = UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A65] = {
        .nId = 286,
        .pModelName = "SLT-A65",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A65V] = {
        .nId = 286,
        .pModelName = "SLT-A65V",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A77] = {
        .nId = 287,
        .pModelName = "SLT-A77",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A77V] = {
        .nId = 287,
        .pModelName = "SLT-A77V",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_5N] = {
        .nId = 288,
        .pModelName = "NEX-5N",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_7] = {
        .nId = 289,
        .pModelName = "NEX-7",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_VG20E] = {
        .nId = 290,
        .pModelName = "NEX-VG20E",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A37] = {
        .nId = 291,
        .pModelName = "SLT-A37",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A57] = {
        .nId = 292,
        .pModelName = "SLT-A57",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_F3] = {
        .nId = 293,
        .pModelName = "NEX-F3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A99] = {
        .nId = 294,
        .pModelName = "SLT-A99",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A99V] = {
        .nId = 294,
        .pModelName = "SLT-A99V",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_6] = {
        .nId = 295,
        .pModelName = "NEX-6",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_5R] = {
        .nId = 296,
        .pModelName = "NEX-5R",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100] = {
        .nId = 297,
        .pModelName = "DSC-RX100",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX1] = {
        .nId = 298,
        .pModelName = "DSC-RX1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_VG900] = {
        .nId = 299,
        .pModelName = "NEX-VG900",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_VG30E] = {
        .nId = 300,
        .pModelName = "NEX-VG30E",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_3000] = {
        .nId = 302,
        .pModelName = "ILCE-3000",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_3500] = {
        .nId = 302,
        .pModelName = "ILCE-3500",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_SLT_A58] = {
        .nId = 303,
        .pModelName = "SLT-A58",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_3N] = {
        .nId = 305,
        .pModelName = "NEX-3N",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7] = {
        .nId = 306,
        .pModelName = "ILCE-7",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_NEX_5T] = {
        .nId = 307,
        .pModelName = "NEX-5T",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M2] = {
        .nId = 308,
        .pModelName = "DSC-RX100M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX10] = {
        .nId = 309,
        .pModelName = "DSC-RX10",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX1R] = {
        .nId = 310,
        .pModelName = "DSC-RX1R",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7R] = {
        .nId = 311,
        .pModelName = "ILCE-7R",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6000] = {
        .nId = 312,
        .pModelName = "ILCE-6000",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_5000] = {
        .nId = 313,
        .pModelName = "ILCE-5000",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M3] = {
        .nId = 317,
        .pModelName = "DSC-RX100M3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7S] = {
        .nId = 318,
        .pModelName = "ILCE-7S",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCA_77M2] = {
        .nId = 319,
        .pModelName = "ILCA-77M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_5100] = {
        .nId = 339,
        .pModelName = "ILCE-5100",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7M2] = {
        .nId = 340,
        .pModelName = "ILCE-7M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M4] = {
        .nId = 341,
        .pModelName = "DSC-RX100M4",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX10M2] = {
        .nId = 342,
        .pModelName = "DSC-RX10M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX1RM2] = {
        .nId = 344,
        .pModelName = "DSC-RX1RM2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_QX1] = {
        .nId = 346,
        .pModelName = "ILCE-QX1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7RM2] = {
        .nId = 347,
        .pModelName = "ILCE-7RM2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7SM2] = {
        .nId = 350,
        .pModelName = "ILCE-7SM2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCA_68] = {
        .nId = 353,
        .pModelName = "ILCA-68",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCA_99M2] = {
        .nId = 354,
        .pModelName = "ILCA-99M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX10M3] = {
        .nId = 355,
        .pModelName = "DSC-RX10M3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M5] = {
        .nId = 356,
        .pModelName = "DSC-RX100M5",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6300] = {
        .nId = 357,
        .pModelName = "ILCE-6300",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_9] = {
        .nId = 358,
        .pModelName = "ILCE-9",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6500] = {
        .nId = 360,
        .pModelName = "ILCE-6500",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7RM3] = {
        .nId = 362,
        .pModelName = "ILCE-7RM3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7M3] = {
        .nId = 363,
        .pModelName = "ILCE-7M3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX0] = {
        .nId = 364,
        .pModelName = "DSC-RX0",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX10M4] = {
        .nId = 365,
        .pModelName = "DSC-RX10M4",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M6] = {
        .nId = 366,
        .pModelName = "DSC-RX100M6",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_HX99] = {
        .nId = 367,
        .pModelName = "DSC-HX99",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M5A] = {
        .nId = 369,
        .pModelName = "DSC-RX100M5A",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6400] = {
        .nId = 371,
        .pModelName = "ILCE-6400",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX0M2] = {
        .nId = 372,
        .pModelName = "DSC-RX0M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_HX95] = {
        .nId = 373,
        .pModelName = "DSC-HX95",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M7] = {
        .nId = 374,
        .pModelName = "DSC-RX100M7",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7RM4] = {
        .nId = 375,
        .pModelName = "ILCE-7RM4",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_9M2] = {
        .nId = 376,
        .pModelName = "ILCE-9M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6600] = {
        .nId = 378,
        .pModelName = "ILCE-6600",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6100] = {
        .nId = 379,
        .pModelName = "ILCE-6100",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_1] = {
        .nId = 380,
        .pModelName = "ZV-1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7C] = {
        .nId = 381,
        .pModelName = "ILCE-7C",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_E10] = {
        .nId = 382,
        .pModelName = "ZV-E10",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7SM3] = {
        .nId = 383,
        .pModelName = "ILCE-7SM3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_1] = {
        .nId = 384,
        .pModelName = "ILCE-1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILME_FX3] = {
        .nId = 385,
        .pModelName = "ILME-FX3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7RM3A] = {
        .nId = 386,
        .pModelName = "ILCE-7RM3A",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7RM4A] = {
        .nId = 387,
        .pModelName = "ILCE-7RM4A",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7M4] = {
        .nId = 388,
        .pModelName = "ILCE-7M4",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_1F] = {
        .nId = 389,
        .pModelName = "ZV-1F",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7RM5] = {
        .nId = 390,
        .pModelName = "ILCE-7RM5",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILME_FX30] = {
        .nId = 391,
        .pModelName = "ILME-FX30",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_9M3] = {
        .nId = 392,
        .pModelName = "ILCE-9M3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_E1] = {
        .nId = 393,
        .pModelName = "ZV-E1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6700] = {
        .nId = 394,
        .pModelName = "ILCE-6700",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_1M2] = {
        .nId = 395,
        .pModelName = "ZV-1M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7CR] = {
        .nId = 396,
        .pModelName = "ILCE-7CR",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_7CM2] = {
        .nId = 397,
        .pModelName = "ILCE-7CM2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILX_LR1] = {
        .nId = 398,
        .pModelName = "ILX-LR1",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_E10M2] = {
        .nId = 399,
        .pModelName = "ZV-E10M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_1M2] = {
        .nId = 400,
        .pModelName = "ILCE-1M2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX1RM3] = {
        .nId = 401,
        .pModelName = "DSC-RX1RM3",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6400A] = {
        .nId = 402,
        .pModelName = "ILCE-6400A",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILCE_6100A] = {
        .nId = 403,
        .pModelName = "ILCE-6100A",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_DSC_RX100M7A] = {
        .nId = 404,
        .pModelName = "DSC-RX100M7A",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ILME_FX2] = {
        .nId = 406,
        .pModelName = "ILME-FX2",
        .ReadCameraInfo = NULL,
    },
    [PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_ID_ZV_1A] = {
        .nId = 408,
        .pModelName = "ZV-1A",
        .ReadCameraInfo = NULL,
    },
};

void UpdateSegmentInfoWindow_APP1_ExifReadSony0010(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    for (PICTUREJPEGMAKERNOTESONYMODELID iLoop = 0; iLoop < PICTURE_JPEG_MAKER_NOTE_SONY_MODEL_CNT; iLoop++)
    {
        if (!_stricmp(pReadExif->ifdData.exInfo.pCameraModel, sg_pSonyModelInfo[iLoop].pModelName))
        {
            if (NULL != sg_pSonyModelInfo[iLoop].ReadCameraInfo)
            {
                sg_pSonyModelInfo[iLoop].ReadCameraInfo(pReadExif);
            }
            else
            {
                HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("相机信息"));
                HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("未收录"));
                UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
            }
        }
    }
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo1(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("相机信息"));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("未收录"));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo2(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("相机信息"));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("未收录"));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfo3(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint8_t* pData = pReadExif->ifdData.pIFDData;
    const int16_t* pValueS16;
    uint8_t cValueU8;
    uint16_t cValueU16;

    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_LENS_SPEC_0010));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, (const PSTR)pData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU16 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[14]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FOCAL_LENGTH));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g毫米"), (HANDOUBLE)cValueU16 / (HANDOUBLE)10);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU16 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[16]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FOCAL_LENGTH_TELE_ZOOM));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g毫米"), (HANDOUBLE)cValueU16);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[28];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_POINT_SELECTED));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cValueU8, GetJpegMakerNoteSonyTagAFPointSelectedName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[29];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FOCUS_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cValueU8, GetJpegMakerNoteSonyTagFocusModeName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[32];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_POINT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cValueU8, GetJpegMakerNoteSonyTagAFPointName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[25];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FOCUS_STATUS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cValueU8, GetJpegMakerNoteSonyTagFocusStatusName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    pData = &pData[33];
    memcpy(&pValueS16, &pData, sizeof(pValueS16));
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_ACTIVE_SENSOR));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[0]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_UPPER_LEFT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[1]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_LEFT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[2]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_LOWER_LEFT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[3]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_FAR_LEFT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[4]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_TOP_HORIZONTAL));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[5]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_NEAR_RIGHT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[6]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_CENTER_HORIZONTAL));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[7]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_NEAR_LEFT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[8]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_BOTTOM_HORIZONTAL));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[9]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_TOP_VERTICAL));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[10]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_CENTER_VERTICAL));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[11]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_BOTTOM_VERTICAL));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[12]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_FAR_RIGHT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[13]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_UPPER_RIGHT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[14]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_RIGHT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[15]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_LOWER_RIGHT));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[16]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_UPPER_MIDDLE));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[17]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_AF_STATUS_LOWER_MIDDLE));
    UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pValueS16[18]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}

static void UpdateSegmentInfoWindow_APP1_ExifReadSony0010CameraInfoPrintAFStatus(HANPSTR pText, size_t nTextSize, int16_t nStatus)
{
    if (-32768 == nStatus) { HAN_snprintf(pText, nTextSize, TEXT("Out of Focus")); }
    else if (0 == nStatus) { HAN_snprintf(pText, nTextSize, TEXT("In Focus")); }
    else if (0 < nStatus) { HAN_snprintf(pText, nTextSize, TEXT("Back Focus (%+d)"), nStatus); }
    else { HAN_snprintf(pText, nTextSize, TEXT("Front Focus (%+d)"), nStatus); }
}
