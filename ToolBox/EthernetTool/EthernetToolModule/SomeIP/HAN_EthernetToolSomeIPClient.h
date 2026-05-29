#ifndef HAN_ETHERNET_TOOL_SOME_IP_CLIENT_H
#define HAN_ETHERNET_TOOL_SOME_IP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_EthernetToolDef.h"
#include "HAN_EthernetToolSomeIPTypedef.h"

void RegisterHANEthernetToolSomeIPClient(HINSTANCE hInst);

HWND CreateEthernetToolSomeIPClientWindow(RECT* rcClient, HWND hEthernetToolSomeIP, HINSTANCE hInst);

#ifdef __cplusplus
}
#endif

#endif
