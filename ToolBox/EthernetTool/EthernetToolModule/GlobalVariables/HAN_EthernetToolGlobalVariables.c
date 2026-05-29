#include "HAN_EthernetToolGlobalVariables.h"

const ETHERNETTOOLEXTRA     g_etDefaultCfg = {
    .cfgEthernet = {
        .local = {
            .pIPv4 = { 127, 0, 0, 1 },
            .nPort = 80,
            .sct = INVALID_SOCKET,
        },
        .remote = {
            .pIPv4 = { 127, 0, 0, 1 },
            .nPort = 80,
            .sct = INVALID_SOCKET,
        },
        .protocol = {
            .eId = ETHERNETTOOL_PROTOCOL_UDP,
        },
        .hSocketThread = NULL,
    },
    .printHex = {
        .bHex = FALSE,
    },
    .sendData = {
        .nBufLen = 0,
    },
};
