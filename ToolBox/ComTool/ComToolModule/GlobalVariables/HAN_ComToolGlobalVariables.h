#ifndef HAN_COM_TOOL_GLOBAL_VARIABLES_H
#define HAN_COM_TOOL_GLOBAL_VARIABLES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_ComToolDef.h"

extern const HANPCSTR       g_pBaudrate[COMTOOL_BAUDRATE_CNT];
extern const HANPCSTR       g_pByteSize[COMTOOL_BYTESIZE_CNT];
extern const COMSBPAPARAM   g_pStopBits[COMTOOL_STOPBITS_CNT];
extern const COMSBPAPARAM   g_pParity[COMTOOL_PARITY_CNT];
extern const COMTOOLEXTRA   g_ftDefaultCfg;

#ifdef __cplusplus
}
#endif

#endif
