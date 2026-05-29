#include "HAN_ComToolGlobalVariables.h"

const HANPCSTR       g_pBaudrate[COMTOOL_BAUDRATE_CNT] = {
    [COMTOOL_BAUDRATE_USER] = TEXT("×Ô¶¨Òå"),
    [COMTOOL_BAUDRATE_110] = TEXT("110"),
    [COMTOOL_BAUDRATE_300] = TEXT("300"),
    [COMTOOL_BAUDRATE_600] = TEXT("600"),
    [COMTOOL_BAUDRATE_1200] = TEXT("1200"),
    [COMTOOL_BAUDRATE_2400] = TEXT("2400"),
    [COMTOOL_BAUDRATE_4800] = TEXT("4800"),
    [COMTOOL_BAUDRATE_9600] = TEXT("9600"),
    [COMTOOL_BAUDRATE_14400] = TEXT("14400"),
    [COMTOOL_BAUDRATE_19200] = TEXT("19200"),
    [COMTOOL_BAUDRATE_38400] = TEXT("38400"),
    [COMTOOL_BAUDRATE_56000] = TEXT("56000"),
    [COMTOOL_BAUDRATE_57600] = TEXT("57600"),
    [COMTOOL_BAUDRATE_115200] = TEXT("115200"),
    [COMTOOL_BAUDRATE_128000] = TEXT("128000"),
    [COMTOOL_BAUDRATE_230400] = TEXT("230400"),
    [COMTOOL_BAUDRATE_256000] = TEXT("256000"),
    [COMTOOL_BAUDRATE_460800] = TEXT("460800"),
    [COMTOOL_BAUDRATE_500000] = TEXT("500000"),
    [COMTOOL_BAUDRATE_512000] = TEXT("512000"),
    [COMTOOL_BAUDRATE_600000] = TEXT("600000"),
    [COMTOOL_BAUDRATE_750000] = TEXT("750000"),
    [COMTOOL_BAUDRATE_921600] = TEXT("921600"),
    [COMTOOL_BAUDRATE_1000000] = TEXT("1000000"),
    [COMTOOL_BAUDRATE_1500000] = TEXT("1500000"),
    [COMTOOL_BAUDRATE_2000000] = TEXT("2000000"),
    [COMTOOL_BAUDRATE_3000000] = TEXT("3000000"),
};
const HANPCSTR       g_pByteSize[COMTOOL_BYTESIZE_CNT] = {
    [COMTOOL_BYTESIZE_5] = TEXT("5"),
    [COMTOOL_BYTESIZE_6] = TEXT("6"),
    [COMTOOL_BYTESIZE_7] = TEXT("7"),
    [COMTOOL_BYTESIZE_8] = TEXT("8"),
};
const COMSBPAPARAM   g_pStopBits[COMTOOL_STOPBITS_CNT] = {
    [COMTOOL_STOPBITS_1] = { .pText = TEXT("1"), .value = ONESTOPBIT, },
    [COMTOOL_STOPBITS_1_5] = { .pText = TEXT("1.5"), .value = ONE5STOPBITS, },
    [COMTOOL_STOPBITS_2] = { .pText = TEXT("2"), .value = TWOSTOPBITS, },
};
const COMSBPAPARAM   g_pParity[COMTOOL_PARITY_CNT] = {
    [COMTOOL_PARITY_NONE] = { .pText = TEXT("NONE"), .value = NOPARITY, },
    [COMTOOL_PARITY_ODD] = { .pText = TEXT("ODD"), .value = ODDPARITY, },
    [COMTOOL_PARITY_EVEN] = { .pText = TEXT("EVEN"), .value = EVENPARITY, },
    [COMTOOL_PARITY_MARK] = { .pText = TEXT("MARK"), .value = MARKPARITY, },
    [COMTOOL_PARITY_SPACE] = { .pText = TEXT("SPACE"), .value = SPACEPARITY, },
};
const COMTOOLEXTRA   g_ftDefaultCfg = {
    .cfgCOM = {
        .cfgCOM = {
            .comFile = {
                .id = 1,
                .dwFlagsAndAttributes = 0,
            },
            .comState = {
                .BaudRate = 115200,
                .ByteSize = 8,
                .Parity = NOPARITY,
                .StopBits = ONESTOPBIT,
            },
            .comBuf = {
                .dwInQueue = 8192,
                .dwOutQueue = 8192,
            },
            .comTime = {
                .ReadIntervalTimeout = 0,
                .ReadTotalTimeoutConstant = 0,
                .ReadTotalTimeoutMultiplier = 0,
                .WriteTotalTimeoutConstant = 0,
                .WriteTotalTimeoutMultiplier = 0,
            },
        },
    },
    .hComFile = INVALID_HANDLE_VALUE,
    .nTextWindowLen = 0,
    .revData.tTime = {
        .msTime = 0,
        .msTimeOut = COMTOOL_DEF_USER_TIMEOUT,
    },
    .printHex = {
        .bHex = FALSE,
    },
    .printData = {
        .printStartTime = {
            .bStart = TRUE,
        },
        .printEndTime = {
            .bEnd = TRUE,
        },
    },
    .sumData = {
        .sumTx.nSum = 0,
        .sumRx.nSum = 0,
    },
    .saveToFile = {
        .bSave = FALSE,
    },
};
