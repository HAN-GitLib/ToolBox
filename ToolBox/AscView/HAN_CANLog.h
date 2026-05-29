#ifndef HAN_CAN_LOG_H
#define HAN_CAN_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CAN_DLC_LEN_MAX     256

typedef double CANLOGTIME;

typedef enum {
    CAN_LOG_TR_TX,
    CAN_LOG_TR_RX,
} CANLOGTR;

typedef struct tagCANLOG {
    CANLOGTIME  tTime;
    uint32_t    nChannel;
    uint32_t    canId;
    CANLOGTR    cTxRx;
    uint32_t    nLen;
    uint8_t     pData[CAN_DLC_LEN_MAX];
} CANLOG, * PCANLOG;
typedef const CANLOG* PCCANLOG;

#ifdef __cplusplus
}
#endif

#endif
