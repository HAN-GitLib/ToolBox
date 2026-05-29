#ifndef HAN_ETHERNET_TOOL_SOME_IP_H
#define HAN_ETHERNET_TOOL_SOME_IP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_EthernetToolDef.h"
#include "HAN_EthernetToolSomeIPTypedef.h"

void RegisterHANEthernetToolSomeIP(HINSTANCE hInst);

HWND CreateEthernetToolSomeIPWindow(PETHERNETTOOLEXTRA etInfo);

#ifdef __cplusplus
}
#endif

#endif
