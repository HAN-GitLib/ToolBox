#ifndef DLL_DEMO_DEF_H
#define DLL_DEMO_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct tagIMUData {
    double ax;
    double ay;
    double az;
    double wx;
    double wy;
    double wz;
    double temperature;
} IMUDATA, * PIMUDATA;
typedef const IMUDATA* PCIMUDATA;

typedef struct tagGNSSData {
    /* GNSS mask */
    uint32_t    cGNSSMask;
    /* RMC */
    uint32_t    rmcUTC;
    float       rmcVel;
    float       rmcHeading;
    /* GGA */
    double      ggaLat;
    double      ggaLon;
    float       ggaAlt;
    int         ggaStlN;
    uint16_t    ggaHDop;
    uint8_t     ggaFlag;
    /* GST */
    float       gstLatStd;
    float       gstLonStd;
    float       gstAltStd;
    /* GSA */
    uint16_t    gsaPDop;
    uint16_t    gsaHDop;
    uint16_t    gsaVDop;
    /* RELPOSNED */
    uint32_t    rpnAccN;
    uint32_t    rpnAccE;
    uint32_t    rpnAccD;
    uint32_t    rpnAccHeading;
    float       rpnHeading;
    uint8_t     rpnCarrSoln;
    /* VELNED */
    int32_t     vnVelN;
    int32_t     vnVelE;
    int32_t     vnVelD;
    uint32_t    vnSAcc;
} GNSSDATA, * PGNSSDATA;
typedef const GNSSDATA* PCGNSSDATA;

typedef struct tagCANDATA {
    uint8_t     cGear;              // 档位，实测里面没有任何有用的数据
    float       nSteerAngle;        // 方向盘转角
    double      nDrivingLWheel;     // 驱动轮左轮转速
    double      nDrivingRWheel;     // 驱动轮右轮转速
    double      nDrivenLWheel;      // 非驱动轮左轮转速
    double      nDrivenRWheel;      // 非驱动轮右轮转速
    uint8_t     cMask;              // 当前CANbus是否收到了相关信号
} CANDATA, * PCANDATA;
typedef const CANDATA* PCCANDATA;

typedef struct tagINSDATA {
    float       dPitch;
    float       dRoll;
    float       dYaw;
    float       msVelE;
    float       msVelN;
    float       msVelU;
    double      dLat;
    double      dLon;
    float       mAlt;
    uint32_t    cSysState;
} INSDATA, * PINSDATA;
typedef const INSDATA* PCINSDATA;

typedef struct tagMASTERMSG {
    IMUDATA         imuData;
    GNSSDATA        gnssData;
    CANDATA         canData;
    INSDATA         insData;
    uint32_t        tIMUTime;
    uint32_t        tSysTime;
    uint32_t        nMsgCnt;
} MASTERMSG;

#ifdef __cplusplus
}
#endif

#endif
