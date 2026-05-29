#ifndef GRAPHIC_DLL_DEMO_H
#define GRAPHIC_DLL_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#define VAR_VALUE_TEXT_SIZE     32

typedef enum {
    VAR_ID_MSG_C403_WhlGndVelLNonDrvnHSC2,
    VAR_ID_MSG_C403_WhlGndVelRNonDrvnVHSC2,
    VAR_ID_MSG_C403_WhlGndVelLDrvnVHSC2,
    VAR_ID_MSG_C403_WhlGndVelRDrvnHSC2,
    VAR_ID_MSG_C403_StrWhlangHSC2,
    VAR_ID_MSG_C403_indicator,
    VAR_ID_MSG_C403_ScuShifterLvrposHSC2,
    VAR_ID_MSG_C403_sendNumber,
    VAR_ID_MSG_C403_UTC,
    VAR_ID_MSG_C403_Heading,
    VAR_ID_MSG_C403_EastVel,
    VAR_ID_MSG_C403_northVel,
    VAR_ID_MSG_C403_Latitude,
    VAR_ID_MSG_C403_Longitude,
    VAR_ID_MSG_C403_height,
    VAR_ID_MSG_C403_latstd,
    VAR_ID_MSG_C403_lonstd,
    VAR_ID_MSG_C403_altstd,
    VAR_ID_MSG_C403_POSHeaing,
    VAR_ID_MSG_C403_solnSVs01,
    VAR_ID_MSG_C403_solnSVs02,
    VAR_ID_MSG_C403_pdop,
    VAR_ID_MSG_C403_phop,
    VAR_ID_MSG_C403_pvop,
    VAR_ID_MSG_C403_acc_n,
    VAR_ID_MSG_C403_acc_e,
    VAR_ID_MSG_C403_acc_d,
    VAR_ID_MSG_C403_acc_heading,
    VAR_ID_MSG_C403_velN,
    VAR_ID_MSG_C403_velE,
    VAR_ID_MSG_C403_velD,
    VAR_ID_MSG_C403_sAcc,
    VAR_ID_MSG_C403_gga_flag,
    VAR_ID_MSG_C403_ACC_X,
    VAR_ID_MSG_C403_ACC_Y,
    VAR_ID_MSG_C403_ACC_Z,
    VAR_ID_MSG_C403_Gyro_X,
    VAR_ID_MSG_C403_Gyro_Y,
    VAR_ID_MSG_C403_Gyro_Z,
    VAR_ID_MSG_C403_IMU_Time,
    VAR_ID_MSG_C403_can_time,
    VAR_ID_MSG_C403_IMU_Temp,
    VAR_ID_MSG_C403_pitch,
    VAR_ID_MSG_C403_roll,
    VAR_ID_MSG_C403_yaw,
    VAR_ID_MSG_C403_EstVel,
    VAR_ID_MSG_C403_NorthVel,
    VAR_ID_MSG_C403_UpVel,
    VAR_ID_MSG_C403_lat,
    VAR_ID_MSG_C403_lon,
    VAR_ID_MSG_C403_alt,
    VAR_ID_MSG_C403_sysStatus,
    VAR_ID_MSG_C403_CNT,
} VARIDMSGC403;
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
typedef struct tagMSGC403 {
    IMUDATA         imuData;
    GNSSDATA        gnssData;
    CANDATA         canData;
    INSDATA         insData;
    uint32_t        tIMUTime;
    uint32_t        tSysTime;
    uint32_t        nMsgCnt;
} MSGC403;

#ifdef __cplusplus
}
#endif

#endif
