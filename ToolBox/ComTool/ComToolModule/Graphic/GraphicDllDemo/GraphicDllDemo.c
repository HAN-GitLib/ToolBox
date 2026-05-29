#include "..\HAN_ComToolGraphicTypedef.h"
#include "GraphicDllDemo.h"

static size_t ReadMsgC403(const uint8_t* pMsg, size_t nMsgLen, PCOMTOOLGRAPHICVAR pVar);
static size_t ReadMessage1(const uint8_t* pMsg, size_t nMsgLen, PCOMTOOLGRAPHICVAR pVar);

static CTGCHAR              sg_pValueMsgC403[VAR_ID_MSG_C403_CNT][VAR_VALUE_TEXT_SIZE];
static COMTOOLGRAPHICVAR    sg_pVarMsgC403[VAR_ID_MSG_C403_CNT] = {
    [VAR_ID_MSG_C403_WhlGndVelLNonDrvnHSC2] = { .pName = TEXT("WhlGndVelLNonDrvnHSC2"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_WhlGndVelLNonDrvnHSC2] },
    [VAR_ID_MSG_C403_WhlGndVelRNonDrvnVHSC2] = { .pName = TEXT("WhlGndVelRNonDrvnVHSC2"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_WhlGndVelRNonDrvnVHSC2] },
    [VAR_ID_MSG_C403_WhlGndVelLDrvnVHSC2] = { .pName = TEXT("WhlGndVelLDrvnVHSC2"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_WhlGndVelLDrvnVHSC2] },
    [VAR_ID_MSG_C403_WhlGndVelRDrvnHSC2] = { .pName = TEXT("WhlGndVelRDrvnHSC2"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_WhlGndVelRDrvnHSC2] },
    [VAR_ID_MSG_C403_StrWhlangHSC2] = { .pName = TEXT("StrWhlangHSC2"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_StrWhlangHSC2] },
    [VAR_ID_MSG_C403_indicator] = { .pName = TEXT("indicator"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_indicator] },
    [VAR_ID_MSG_C403_ScuShifterLvrposHSC2] = { .pName = TEXT("ScuShifterLvrposHSC2"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_ScuShifterLvrposHSC2] },
    [VAR_ID_MSG_C403_sendNumber] = { .pName = TEXT("sendNumber"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_sendNumber] },
    [VAR_ID_MSG_C403_UTC] = { .pName = TEXT("UTC"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_UTC] },
    [VAR_ID_MSG_C403_Heading] = { .pName = TEXT("Heading"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_Heading] },
    [VAR_ID_MSG_C403_EastVel] = { .pName = TEXT("EastVel"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_EastVel] },
    [VAR_ID_MSG_C403_northVel] = { .pName = TEXT("northVel"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_northVel] },
    [VAR_ID_MSG_C403_Latitude] = { .pName = TEXT("Latitude"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_Latitude] },
    [VAR_ID_MSG_C403_Longitude] = { .pName = TEXT("Longitude"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_Longitude] },
    [VAR_ID_MSG_C403_height] = { .pName = TEXT("height"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_height] },
    [VAR_ID_MSG_C403_latstd] = { .pName = TEXT("latstd"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_latstd] },
    [VAR_ID_MSG_C403_lonstd] = { .pName = TEXT("lonstd"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_lonstd] },
    [VAR_ID_MSG_C403_altstd] = { .pName = TEXT("altstd"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_altstd] },
    [VAR_ID_MSG_C403_POSHeaing] = { .pName = TEXT("POSHeaing"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_POSHeaing] },
    [VAR_ID_MSG_C403_solnSVs01] = { .pName = TEXT("solnSVs01"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_solnSVs01] },
    [VAR_ID_MSG_C403_solnSVs02] = { .pName = TEXT("solnSVs02"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_solnSVs02] },
    [VAR_ID_MSG_C403_pdop] = { .pName = TEXT("pdop"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_pdop] },
    [VAR_ID_MSG_C403_phop] = { .pName = TEXT("phop"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_phop] },
    [VAR_ID_MSG_C403_pvop] = { .pName = TEXT("pvop"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_pvop] },
    [VAR_ID_MSG_C403_acc_n] = { .pName = TEXT("acc_n"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_acc_n] },
    [VAR_ID_MSG_C403_acc_e] = { .pName = TEXT("acc_e"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_acc_e] },
    [VAR_ID_MSG_C403_acc_d] = { .pName = TEXT("acc_d"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_acc_d] },
    [VAR_ID_MSG_C403_acc_heading] = { .pName = TEXT("acc_heading"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_acc_heading] },
    [VAR_ID_MSG_C403_velN] = { .pName = TEXT("velN"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_velN] },
    [VAR_ID_MSG_C403_velE] = { .pName = TEXT("velE"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_velE] },
    [VAR_ID_MSG_C403_velD] = { .pName = TEXT("velD"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_velD] },
    [VAR_ID_MSG_C403_sAcc] = { .pName = TEXT("sAcc"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_sAcc] },
    [VAR_ID_MSG_C403_gga_flag] = { .pName = TEXT("gga_flag"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_gga_flag] },
    [VAR_ID_MSG_C403_ACC_X] = { .pName = TEXT("ACC_X"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_ACC_X] },
    [VAR_ID_MSG_C403_ACC_Y] = { .pName = TEXT("ACC_Y"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_ACC_Y] },
    [VAR_ID_MSG_C403_ACC_Z] = { .pName = TEXT("ACC_Z"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_ACC_Z] },
    [VAR_ID_MSG_C403_Gyro_X] = { .pName = TEXT("Gyro_X"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_Gyro_X] },
    [VAR_ID_MSG_C403_Gyro_Y] = { .pName = TEXT("Gyro_Y"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_Gyro_Y] },
    [VAR_ID_MSG_C403_Gyro_Z] = { .pName = TEXT("Gyro_Z"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_Gyro_Z] },
    [VAR_ID_MSG_C403_IMU_Time] = { .pName = TEXT("IMU_Time"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_IMU_Time] },
    [VAR_ID_MSG_C403_can_time] = { .pName = TEXT("can_time"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_can_time] },
    [VAR_ID_MSG_C403_IMU_Temp] = { .pName = TEXT("IMU_Temp"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_IMU_Temp] },
    [VAR_ID_MSG_C403_pitch] = { .pName = TEXT("pitch"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_pitch] },
    [VAR_ID_MSG_C403_roll] = { .pName = TEXT("roll"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_roll] },
    [VAR_ID_MSG_C403_yaw] = { .pName = TEXT("yaw"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_yaw] },
    [VAR_ID_MSG_C403_EstVel] = { .pName = TEXT("EstVel"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_EstVel] },
    [VAR_ID_MSG_C403_NorthVel] = { .pName = TEXT("NorthVel"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_NorthVel] },
    [VAR_ID_MSG_C403_UpVel] = { .pName = TEXT("UpVel"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_UpVel] },
    [VAR_ID_MSG_C403_lat] = { .pName = TEXT("lat"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_lat] },
    [VAR_ID_MSG_C403_lon] = { .pName = TEXT("lon"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_lon] },
    [VAR_ID_MSG_C403_alt] = { .pName = TEXT("alt"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_alt] },
    [VAR_ID_MSG_C403_sysStatus] = { .pName = TEXT("sysStatus"), .pValue = sg_pValueMsgC403[VAR_ID_MSG_C403_sysStatus] },
};

static COMTOOLGRAPHICVAR    sg_pVarMsg1[] = {
    { .pName = TEXT("Msg1Var1") },
    { .pName = TEXT("Msg1Var2") },
    { .pName = TEXT("Msg1Var3") },
};

static COMTOOLGRAPHICMSG   sg_pComToolGraphic[] = {
    {
        .pMsgName = TEXT("Car"),
        .pVar = sg_pVarMsgC403,
        .nVarCnt = VAR_ID_MSG_C403_CNT,
        .ReadMessage = ReadMsgC403,
    },
    {
        .pMsgName = TEXT("Msg1"),
        .pVar = sg_pVarMsg1,
        .nVarCnt = ARRAYSIZE(sg_pVarMsg1),
        .ReadMessage = ReadMessage1,
    },
};

COMTOOLGRAPHICMSGINFO      g_pComToolGraphicInfo = {
    .pMsg = sg_pComToolGraphic,
    .nMsgCnt = ARRAYSIZE(sg_pComToolGraphic),
};

static uint32_t HY_CRC16(const uint8_t* data, size_t len)
{
    uint32_t ret = (0xFFFF ^ 0x8005) << 8;
    uint32_t poly24 = (0x8005 << 8) | 0x1000000;
    for (size_t i = 0; i < len; i++)
    {
        ret |= data[i];
        for (size_t j = 0; j < 8; j++)
        {
            ret <<= 1;
            if ((ret & 0x1000000))
                ret ^= poly24;
        }
    }
    return ((ret >> 8) & 0xFFFF);
}
static size_t ReadMsgC403(const uint8_t* pMsg, size_t nMsgLen, PCOMTOOLGRAPHICVAR pVar)
{
    uint32_t nRet = 257;
    uint16_t nLen;
    MSGC403 mmMsg;
    
    if (nMsgLen < nRet) { nRet = 0; }
    if ((pMsg[0] != 0xC4) || (pMsg[1] != 0x03)) { nRet = 0; }
    if (nRet > 0)
    {
        pMsg += 2;
        memcpy(&nLen, pMsg + 1, sizeof(nLen));
        if ((nRet - 5) != nLen) { nRet = 0; }

        if (0 == nRet || 0 != HY_CRC16(pMsg, nLen + 3)) { nRet = 0; }
        
        if (nRet > 0)
        {
            pMsg += 3;
            memcpy(&(mmMsg.insData.dPitch), pMsg, sizeof(mmMsg.insData.dPitch)); pMsg += sizeof(mmMsg.insData.dPitch);
            memcpy(&(mmMsg.insData.dRoll), pMsg, sizeof(mmMsg.insData.dRoll)); pMsg += sizeof(mmMsg.insData.dRoll);
            memcpy(&(mmMsg.insData.dYaw), pMsg, sizeof(mmMsg.insData.dYaw)); pMsg += sizeof(mmMsg.insData.dYaw);
            memcpy(&(mmMsg.insData.msVelE), pMsg, sizeof(mmMsg.insData.msVelE)); pMsg += sizeof(mmMsg.insData.msVelE);
            memcpy(&(mmMsg.insData.msVelN), pMsg, sizeof(mmMsg.insData.msVelN)); pMsg += sizeof(mmMsg.insData.msVelN);
            memcpy(&(mmMsg.insData.msVelU), pMsg, sizeof(mmMsg.insData.msVelU)); pMsg += sizeof(mmMsg.insData.msVelU);
            memcpy(&(mmMsg.insData.dLat), pMsg, sizeof(mmMsg.insData.dLat)); pMsg += sizeof(mmMsg.insData.dLat);
            memcpy(&(mmMsg.insData.dLon), pMsg, sizeof(mmMsg.insData.dLon)); pMsg += sizeof(mmMsg.insData.dLon);
            memcpy(&(mmMsg.insData.mAlt), pMsg, sizeof(mmMsg.insData.mAlt)); pMsg += sizeof(mmMsg.insData.mAlt);
            memcpy(&(mmMsg.insData.cSysState), pMsg, sizeof(mmMsg.insData.cSysState)); pMsg += sizeof(mmMsg.insData.cSysState);
            memcpy(&(mmMsg.gnssData.rmcUTC), pMsg, sizeof(mmMsg.gnssData.rmcUTC)); pMsg += sizeof(mmMsg.gnssData.rmcUTC);
            memcpy(&(mmMsg.gnssData.rpnHeading), pMsg, sizeof(mmMsg.gnssData.rpnHeading)); pMsg += sizeof(mmMsg.gnssData.rpnHeading);
            pMsg += sizeof(float);
            memcpy(&(mmMsg.gnssData.rmcVel), pMsg, sizeof(mmMsg.gnssData.rmcVel)); pMsg += sizeof(mmMsg.gnssData.rmcVel);
            memcpy(&(mmMsg.gnssData.ggaLat), pMsg, sizeof(mmMsg.gnssData.ggaLat)); pMsg += sizeof(mmMsg.gnssData.ggaLat);
            memcpy(&(mmMsg.gnssData.ggaLon), pMsg, sizeof(mmMsg.gnssData.ggaLon)); pMsg += sizeof(mmMsg.gnssData.ggaLon);
            memcpy(&(mmMsg.gnssData.ggaAlt), pMsg, sizeof(mmMsg.gnssData.ggaAlt)); pMsg += sizeof(mmMsg.gnssData.ggaAlt);
            memcpy(&(mmMsg.gnssData.gstLatStd), pMsg, sizeof(mmMsg.gnssData.gstLatStd)); pMsg += sizeof(mmMsg.gnssData.gstLatStd);
            memcpy(&(mmMsg.gnssData.gstLonStd), pMsg, sizeof(mmMsg.gnssData.gstLonStd)); pMsg += sizeof(mmMsg.gnssData.gstLonStd);
            memcpy(&(mmMsg.gnssData.gstAltStd), pMsg, sizeof(mmMsg.gnssData.gstAltStd)); pMsg += sizeof(mmMsg.gnssData.gstAltStd);
            memcpy(&(mmMsg.gnssData.rpnCarrSoln), pMsg, sizeof(mmMsg.gnssData.rpnCarrSoln)); pMsg += sizeof(mmMsg.gnssData.rpnCarrSoln);
            memcpy(&(mmMsg.gnssData.ggaStlN), pMsg, sizeof(mmMsg.gnssData.ggaStlN)); pMsg += sizeof(mmMsg.gnssData.ggaStlN);
            pMsg += sizeof(int);
            memcpy(&(mmMsg.gnssData.gsaPDop), pMsg, sizeof(mmMsg.gnssData.gsaPDop)); pMsg += sizeof(mmMsg.gnssData.gsaPDop);
            memcpy(&(mmMsg.gnssData.gsaHDop), pMsg, sizeof(mmMsg.gnssData.gsaHDop)); pMsg += sizeof(mmMsg.gnssData.gsaHDop);
            memcpy(&(mmMsg.gnssData.gsaVDop), pMsg, sizeof(mmMsg.gnssData.gsaVDop)); pMsg += sizeof(mmMsg.gnssData.gsaVDop);
            memcpy(&(mmMsg.gnssData.rpnAccN), pMsg, sizeof(mmMsg.gnssData.rpnAccN)); pMsg += sizeof(mmMsg.gnssData.rpnAccN);
            memcpy(&(mmMsg.gnssData.rpnAccE), pMsg, sizeof(mmMsg.gnssData.rpnAccE)); pMsg += sizeof(mmMsg.gnssData.rpnAccE);
            memcpy(&(mmMsg.gnssData.rpnAccD), pMsg, sizeof(mmMsg.gnssData.rpnAccD)); pMsg += sizeof(mmMsg.gnssData.rpnAccD);
            memcpy(&(mmMsg.gnssData.rpnAccHeading), pMsg, sizeof(mmMsg.gnssData.rpnAccHeading)); pMsg += sizeof(mmMsg.gnssData.rpnAccHeading);
            memcpy(&(mmMsg.gnssData.vnVelN), pMsg, sizeof(mmMsg.gnssData.vnVelN)); pMsg += sizeof(mmMsg.gnssData.vnVelN);
            memcpy(&(mmMsg.gnssData.vnVelE), pMsg, sizeof(mmMsg.gnssData.vnVelE)); pMsg += sizeof(mmMsg.gnssData.vnVelE);
            memcpy(&(mmMsg.gnssData.vnVelD), pMsg, sizeof(mmMsg.gnssData.vnVelD)); pMsg += sizeof(mmMsg.gnssData.vnVelD);
            memcpy(&(mmMsg.gnssData.vnSAcc), pMsg, sizeof(mmMsg.gnssData.vnSAcc)); pMsg += sizeof(mmMsg.gnssData.vnSAcc);
            memcpy(&(mmMsg.gnssData.ggaFlag), pMsg, sizeof(mmMsg.gnssData.ggaFlag)); pMsg += sizeof(mmMsg.gnssData.ggaFlag);
            memcpy(&(mmMsg.imuData.ax), pMsg, sizeof(mmMsg.imuData.ax)); pMsg += sizeof(mmMsg.imuData.ax);
            memcpy(&(mmMsg.imuData.ay), pMsg, sizeof(mmMsg.imuData.ay)); pMsg += sizeof(mmMsg.imuData.ay);
            memcpy(&(mmMsg.imuData.az), pMsg, sizeof(mmMsg.imuData.az)); pMsg += sizeof(mmMsg.imuData.az);
            memcpy(&(mmMsg.imuData.wx), pMsg, sizeof(mmMsg.imuData.wx)); pMsg += sizeof(mmMsg.imuData.wx);
            memcpy(&(mmMsg.imuData.wy), pMsg, sizeof(mmMsg.imuData.wy)); pMsg += sizeof(mmMsg.imuData.wy);
            memcpy(&(mmMsg.imuData.wz), pMsg, sizeof(mmMsg.imuData.wz)); pMsg += sizeof(mmMsg.imuData.wz);
            memcpy(&(mmMsg.tIMUTime), pMsg, sizeof(mmMsg.tIMUTime)); pMsg += sizeof(mmMsg.tIMUTime);
            memcpy(&(mmMsg.tSysTime), pMsg, sizeof(mmMsg.tSysTime)); pMsg += sizeof(mmMsg.tSysTime);
            memcpy(&(mmMsg.imuData.temperature), pMsg, sizeof(mmMsg.imuData.temperature)); pMsg += sizeof(mmMsg.imuData.temperature);
            memcpy(&(mmMsg.canData.nDrivenLWheel), pMsg, sizeof(mmMsg.canData.nDrivenLWheel)); pMsg += sizeof(mmMsg.canData.nDrivenLWheel);
            memcpy(&(mmMsg.canData.nDrivenRWheel), pMsg, sizeof(mmMsg.canData.nDrivenRWheel)); pMsg += sizeof(mmMsg.canData.nDrivenRWheel);
            memcpy(&(mmMsg.canData.nDrivingLWheel), pMsg, sizeof(mmMsg.canData.nDrivingLWheel)); pMsg += sizeof(mmMsg.canData.nDrivingLWheel);
            memcpy(&(mmMsg.canData.nDrivingRWheel), pMsg, sizeof(mmMsg.canData.nDrivingRWheel)); pMsg += sizeof(mmMsg.canData.nDrivingRWheel);
            memcpy(&(mmMsg.canData.nSteerAngle), pMsg, sizeof(mmMsg.canData.nSteerAngle)); pMsg += sizeof(mmMsg.canData.nSteerAngle);
            memcpy(&(mmMsg.canData.cMask), pMsg, sizeof(mmMsg.canData.cMask)); pMsg += sizeof(mmMsg.canData.cMask);
            memcpy(&(mmMsg.canData.cGear), pMsg, sizeof(mmMsg.canData.cGear)); pMsg += sizeof(mmMsg.canData.cGear);
            memcpy(&(mmMsg.nMsgCnt), pMsg, sizeof(mmMsg.nMsgCnt)); pMsg += sizeof(mmMsg.nMsgCnt);

            Ctg_snprintf(pVar[VAR_ID_MSG_C403_WhlGndVelLNonDrvnHSC2].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.canData.nDrivenLWheel);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_WhlGndVelRNonDrvnVHSC2].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.canData.nDrivenRWheel);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_WhlGndVelLDrvnVHSC2].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.canData.nDrivingLWheel);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_WhlGndVelRDrvnHSC2].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.canData.nDrivingRWheel);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_StrWhlangHSC2].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.canData.nSteerAngle);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_indicator].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.canData.cMask);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_ScuShifterLvrposHSC2].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.canData.cGear);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_sendNumber].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.nMsgCnt);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_UTC].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.rmcUTC);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_Heading].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.rpnHeading);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_EastVel].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.vnVelE);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_northVel].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.vnVelN);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_Latitude].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%.15lf"), mmMsg.gnssData.ggaLat);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_Longitude].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%.15lf"), mmMsg.gnssData.ggaLon);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_height].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.ggaAlt);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_latstd].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.gstLatStd);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_lonstd].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.gstLonStd);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_altstd].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.gnssData.gstAltStd);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_POSHeaing].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.rpnCarrSoln);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_solnSVs01].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.ggaStlN);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_solnSVs02].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), 0);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_pdop].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.gsaPDop);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_phop].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.gsaHDop);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_pvop].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.gsaVDop);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_acc_n].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.rpnAccN);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_acc_e].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.rpnAccE);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_acc_d].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.rpnAccD);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_acc_heading].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.rpnAccHeading);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_velN].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%d"), mmMsg.gnssData.vnVelN);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_velE].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%d"), mmMsg.gnssData.vnVelE);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_velD].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%d"), mmMsg.gnssData.vnVelD);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_sAcc].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.vnSAcc);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_gga_flag].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.gnssData.ggaFlag);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_ACC_X].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.ax);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_ACC_Y].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.ay);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_ACC_Z].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.az);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_Gyro_X].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.wx);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_Gyro_Y].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.wy);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_Gyro_Z].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.wz);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_IMU_Time].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.tIMUTime/1000.0);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_can_time].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%u"), mmMsg.tSysTime);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_IMU_Temp].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.imuData.temperature);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_pitch].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.dPitch);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_roll].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.dRoll);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_yaw].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.dYaw);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_EstVel].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.msVelE);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_NorthVel].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.msVelN);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_UpVel].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.msVelU);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_lat].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%.15lf"), mmMsg.insData.dLat);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_lon].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%.15lf"), mmMsg.insData.dLon);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_alt].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%g"), mmMsg.insData.mAlt);
            Ctg_snprintf(pVar[VAR_ID_MSG_C403_sysStatus].pValue, VAR_VALUE_TEXT_SIZE, TEXT("%X"), mmMsg.insData.cSysState);
        }
    }

    return nRet;
}
static size_t ReadMessage1(const uint8_t* pMsg, size_t nMsgLen, PCOMTOOLGRAPHICVAR pVar)
{
    return 0;
}
