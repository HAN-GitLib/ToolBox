#include "ToolBoxInfo.h"
#include "FileConversion\HAN_FileConversionWindow.h"
#include "HexView\HAN_Hex.h"
#include "BinView\HAN_Bin.h"
#include "SrecView\HAN_Srec.h"
#include "AscView\HAN_Asc.h"
#include "ComTool\HAN_ComTool.h"
#include "EthernetTool\HAN_EthernetTool.h"
#include "Picture\HAN_Picture.h"
#include "Video\HAN_Video.h"

static FILECONVERSIONCFG    sg_cfgFileConversion;
static BINVIEWCFG           sg_cfgBinView;
static HEXVIEWCFG           sg_cfgHexView;
static SRECVIEWCFG          sg_cfgSrecView;
static COMTOOLCFG           sg_cfgComTool;
static ETHERNETTOOLCFG      sg_cfgEthernetTool;

TOOLBOXINFO    g_tiTabInfo[TOOL_BOX_ID_CNT_MAX] = {
    [TOOL_BOX_ID_FILE_CONVERSION] = {
        .pTitle = TEXT("文件转换"),
        .pClass = HAN_FILE_CONVERSION_CLASS,
        .RegisterHANTabWindow = RegisterHANFileConversion,
        .ReadIniFile = ReadFileConversionIniFile,
        .WriteIniFile = WriteFileConversionIniFile,
        .pIni = &sg_cfgFileConversion,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_BIN_VIEW] = {
        .pTitle = TEXT("bin"),
        .pClass = HAN_BIN_VIEW_CLASS,
        .RegisterHANTabWindow = RegisterHANBinView,
        .ReadIniFile = ReadBinViewIniFile,
        .WriteIniFile = WriteBinViewIniFile,
        .pIni = &sg_cfgBinView,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_HEX_VIEW] = {
        .pTitle = TEXT("hex"),
        .pClass = HAN_HEX_VIEW_CLASS,
        .RegisterHANTabWindow = RegisterHANHexView,
        .ReadIniFile = ReadHexViewIniFile,
        .WriteIniFile = WriteHexViewIniFile,
        .pIni = &sg_cfgHexView,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_SREC_VIEW] = {
        .pTitle = TEXT("srec"),
        .pClass = HAN_SREC_VIEW_CLASS,
        .RegisterHANTabWindow = RegisterHANSrecView,
        .ReadIniFile = ReadSrecViewIniFile,
        .WriteIniFile = WriteSrecViewIniFile,
        .pIni = &sg_cfgSrecView,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_ASC_VIEW] = {
        .pTitle = TEXT("asc"),
        .pClass = HAN_ASC_VIEW_CLASS,
        .RegisterHANTabWindow = RegisterHANAscView,
        .ReadIniFile = NULL,
        .WriteIniFile = NULL,
        .pIni = NULL,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_COM_TOOL] = {
        .pTitle = TEXT("串口工具"),
        .pClass = HAN_COM_TOOL_CLASS,
        .RegisterHANTabWindow = RegisterHANComTool,
        .ReadIniFile = ReadComToolIniFile,
        .WriteIniFile = WriteComToolIniFile,
        .pIni = &sg_cfgComTool,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_ETHERNET_TOOL] = {
        .pTitle = TEXT("以太网工具"),
        .pClass = HAN_ETHERNET_TOOL_CLASS,
        .RegisterHANTabWindow = RegisterEthernetTool,
        .ReadIniFile = ReadEthernetToolIniFile,
        .WriteIniFile = WriteEthernetToolIniFile,
        .pIni = &sg_cfgEthernetTool,
        .TranslateAcceleratorCallback = NULL,
    },
    [TOOL_BOX_ID_PICTURE] = {
        .pTitle = TEXT("图片处理"),
        .pClass = HAN_PICTURE_CLASS,
        .RegisterHANTabWindow = RegisterHANPicture,
        .ReadIniFile = NULL,
        .WriteIniFile = NULL,
        .pIni = NULL,
        .TranslateAcceleratorCallback = TranslatePictureAccelerator,
    },
    [TOOL_BOX_ID_VIDEO] = {
        .pTitle = TEXT("视频处理"),
        .pClass = HAN_VIDEO_CLASS,
        .RegisterHANTabWindow = RegisterHANVideo,
        .ReadIniFile = NULL,
        .WriteIniFile = NULL,
        .pIni = NULL,
        .TranslateAcceleratorCallback = NULL,
    },
};
