#ifndef HAN_TOOL_BOX_INFO_H
#define HAN_TOOL_BOX_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ToolBoxTypedef.h"
#include "FileConversion\HAN_FileConversionWindow.h"
#include "HexView\HAN_Hex.h"
#include "BinView\HAN_Bin.h"
#include "SrecView\HAN_Srec.h"
#include "AscView\HAN_Asc.h"
#include "ComTool\HAN_ComTool.h"

typedef enum {
    TOOL_BOX_ID_FILE_CONVERSION,
    TOOL_BOX_ID_HEX_VIEW,
    TOOL_BOX_ID_BIN_VIEW,
    TOOL_BOX_ID_SREC_VIEW,
    TOOL_BOX_ID_ASC_VIEW,
    TOOL_BOX_ID_COM_TOOL,
    TOOL_BOX_ID_ETHERNET_TOOL,
    TOOL_BOX_ID_PICTURE,
    TOOL_BOX_ID_VIDEO,
    TOOL_BOX_ID_CNT_MAX,
} TOOLBOXID;

extern TOOLBOXINFO         g_tiTabInfo[TOOL_BOX_ID_CNT_MAX];

#ifdef __cplusplus
}
#endif

#endif
