#ifndef HAN_ETHERNET_TOOL_SOME_IP_SERVER_H
#define HAN_ETHERNET_TOOL_SOME_IP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_EthernetToolDef.h"
#include "HAN_EthernetToolSomeIPTypedef.h"

void RegisterHANEthernetToolSomeIPServer(HINSTANCE hInst);

HWND CreateEthernetToolSomeIPServerWindow(RECT* rcClient, HWND hEthernetToolSomeIP, HINSTANCE hInst);

#ifdef __cplusplus
}
#endif

#endif
