#include <tchar.h>
#include <string.h>
#include <windows.h>
#include <shlwapi.h>

#include "..\HAN_FileConversionTypedef.h"
#include "DllDemoDef.h"

#define TEXT_LINE_CHAR_CNT_MAX      1024

typedef enum {
    WID_USERSETTING_OK,
    WID_USERSETTING_DEST_PATH,
} WID_USERSETTING;

typedef enum {
    FILE_CONVERSION_DLL_DEMO_PROTOCOL_C403,
    FILE_CONVERSION_DLL_DEMO_PROTOCOL_CNT,
} FILECONVERSIONPROTOCOLID;

typedef enum {
    FILE_CONVERSION_DLL_DEMO_CFG_DEST_PATH,
    FILE_CONVERSION_DLL_DEMO_CFG_CNT,
} FILECONVERSIONCFGID;

typedef struct tagDLLDOMESETTINGDATA {
    HWND        hOk;
    HWND        hDestPath;
    FCCHAR      pDestPath[MAX_PATH];
} DLLDOMESETTINGDATA, * PDLLDOMESETTINGDATA;

static bool OpenSrcFile(PCFCSTR pSrcName, PFCSTR pDestName);

static uint32_t Protocol_C403(const uint8_t* pMsg, int nMsgLen, PUSERSETTING pUserSetting, HANDLE hDestFile);

static void InitUserSetting(PUSERSETTING pUserSetting, HWND hParentWnd, HINSTANCE hInst);
static INT_PTR CALLBACK UserSettingDialogProc(HWND hUserSetting, UINT message, WPARAM wParam, LPARAM lParam);
static void SaveUserSetting(PUSERSETTING pUserSetting);

FILECONVERSIONINFO g_pFileConversionInfo = {
    .OpenSrcFileAction = OpenSrcFile,
    .pFileConversion = {
        [FILE_CONVERSION_DLL_DEMO_PROTOCOL_C403] = {
            .pMsgName = TEXT("Car"),
            .pTitle = TEXT(
                "WhlGndVelLNonDrvnHSC2,WhlGndVelRNonDrvnVHSC2,WhlGndVelLDrvnVHSC2,WhlGndVelRDrvnHSC2,"
                "StrWhlangHSC2,indicator,ScuShifterLvrposHSC2,sendNumber,"
                "UTC,Heading,EastVel,northVel,Latitude,Longitude,height,"
                "latstd,lonstd,altstd,POSHeaing,solnSVs01,solnSVs02,"
                "pdop,phop,pvop,"
                "acc_n,acc_e,acc_d,acc_heading,velN,velE,velD,sAcc,"
                "gga_flag,"
                "ACC_X,ACC_Y,ACC_Z,Gyro_X,Gyro_Y,Gyro_Z,IMU_Time,"
                "can_time,IMU_Temp,"
                "pitch,roll,yaw,"
                "EstVel,NorthVel,UpVel,"
                "lat,lon,alt,"
                "sysStatus\r\n"),
            .ReadMessage = Protocol_C403,
        },
    },
    .nFileConversionCnt = FILE_CONVERSION_DLL_DEMO_PROTOCOL_CNT,
    .fcUserSetting = {
        .InitUserSetting = InitUserSetting,
        .UserSettingDialogProc = UserSettingDialogProc,
        .rcDialog = { .left = 100, .top = 100, .right = 450, .bottom = 200, },
        .SaveUserSetting = SaveUserSetting,
    }
};

static bool OpenSrcFile(PCFCSTR pSrcName, PFCSTR pDestName)
{
    bool bRet = false;
    PFCSTR pExt;
    FCCHAR pDestPath[PATH_MAX];

    _tcscpy(pDestName, pSrcName);
    pExt = PathFindExtension(pDestName);
    if (0 != _tcscmp(pExt, TEXT(".csv")))
    {
        _tcscpy(pExt, TEXT(".csv"));
        bRet = true;
    }

    return bRet;
}

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
static uint32_t Protocol_C403(const uint8_t* pMsg, int nMsgLen, PUSERSETTING pUserSetting, HANDLE hDestFile)
{
    (void)pUserSetting;
    uint32_t nRet = 257;
    uint16_t nLen;
    MASTERMSG mmMsg;
    FCCHAR pText[TEXT_LINE_CHAR_CNT_MAX];
    int nWriteLen;

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

            nWriteLen = Fc_snprintf(pText, TEXT_LINE_CHAR_CNT_MAX, TEXT(
                "%g,%g,"
                "%g,%g,"
                "%g,%u,%u,%u,"
                "%u,%g,%g,%g,"
                "%.15lf,%.15lf,%g,"
                "%g,%g,%g,"
                "%u,%u,%u,"
                "%u,%u,%u,"
                "%u,%u,%u,"
                "%u,%d,%d,%d,"
                "%u,%u,"
                "%g,%g,%g,%g,%g,%g,"
                "%g,%u,%g,"
                "%g,%g,%g,"
                "%g,%g,%g,"
                "%.15lf,%.15lf,%g,"
                "%X\n"),
                mmMsg.canData.nDrivenLWheel, mmMsg.canData.nDrivenRWheel,
                mmMsg.canData.nDrivingLWheel, mmMsg.canData.nDrivingRWheel,
                mmMsg.canData.nSteerAngle, mmMsg.canData.cMask, mmMsg.canData.cGear, mmMsg.nMsgCnt,
                mmMsg.gnssData.rmcUTC, mmMsg.gnssData.rpnHeading, mmMsg.gnssData.vnVelE, mmMsg.gnssData.vnVelN,
                mmMsg.gnssData.ggaLat, mmMsg.gnssData.ggaLon, mmMsg.gnssData.ggaAlt,
                mmMsg.gnssData.gstLatStd, mmMsg.gnssData.gstLonStd, mmMsg.gnssData.gstAltStd,
                mmMsg.gnssData.rpnCarrSoln, mmMsg.gnssData.ggaStlN, 0,
                mmMsg.gnssData.gsaPDop, mmMsg.gnssData.gsaHDop, mmMsg.gnssData.gsaVDop,
                mmMsg.gnssData.rpnAccN, mmMsg.gnssData.rpnAccE, mmMsg.gnssData.rpnAccD,
                mmMsg.gnssData.rpnAccHeading, mmMsg.gnssData.vnVelN, mmMsg.gnssData.vnVelE, mmMsg.gnssData.vnVelD,
                mmMsg.gnssData.vnSAcc, mmMsg.gnssData.ggaFlag,
                mmMsg.imuData.ax, mmMsg.imuData.ay, mmMsg.imuData.az, mmMsg.imuData.wx, mmMsg.imuData.wy, mmMsg.imuData.wz,
                mmMsg.tIMUTime / 1000.0, mmMsg.tSysTime, mmMsg.imuData.temperature,
                mmMsg.insData.dPitch, mmMsg.insData.dRoll, mmMsg.insData.dYaw,
                mmMsg.insData.msVelE, mmMsg.insData.msVelN, mmMsg.insData.msVelU,
                mmMsg.insData.dLat, mmMsg.insData.dLon, mmMsg.insData.mAlt,
                mmMsg.insData.cSysState
            );
            WriteFile(hDestFile, pText, nWriteLen, NULL, NULL);
        }
    }

    return nRet;
}

static void InitUserSetting(PUSERSETTING pUserSetting, HWND hParentWnd, HINSTANCE hInst)
{
    PDLLDOMESETTINGDATA pData = GlobalAlloc(0, sizeof(DLLDOMESETTINGDATA));

    if (NULL != pData)
    {
        if (FILE_CONVERSION_DLL_DEMO_CFG_CNT == pUserSetting->nCfgCnt)
        {
            (void)memcpy(
                pData->pDestPath,
                pUserSetting->pCfg[FILE_CONVERSION_DLL_DEMO_CFG_DEST_PATH],
                MAX_PATH
            );
        }
        pUserSetting->pData = pData;
    }
}
static INT_PTR CALLBACK UserSettingDialogProc(HWND hUserSetting, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nRet = TRUE;

    PDLLDOMESETTINGDATA usInfo = (PDLLDOMESETTINGDATA)GetWindowLongPtr(hUserSetting, DWLP_USER);
    
    switch (message) {
        case WM_INITDIALOG: {
            PDLLDOMESETTINGDATA pData = ((PUSERSETTING)lParam)->pData;
            int nWinY = 5;

            pData->hDestPath = CreateWindow(TEXT("edit"), NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER,
                5, nWinY,
                500, 24,
                hUserSetting, (HMENU)WID_USERSETTING_DEST_PATH, NULL, NULL);
            nWinY += 34;
            pData->hOk = CreateWindow(TEXT("button"), TEXT("х╥хо"),
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                10, nWinY,
                100, 32,
                hUserSetting, (HMENU)WID_USERSETTING_OK, NULL, NULL);

            SetWindowText(pData->hDestPath, pData->pDestPath);

            SetWindowLongPtr(hUserSetting, DWLP_USER, (LONG_PTR)pData);
        } break;
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case WID_USERSETTING_OK: {
                    GetWindowText(usInfo->hDestPath, usInfo->pDestPath, MAX_PATH);
                    EndDialog(hUserSetting, 0);
                } break;

                default: {
                } break;
            }
        } break;
        case WM_CLOSE: {
            EndDialog(hUserSetting, 0);
        } break;
        case WM_DESTROY: {
            EndDialog(hUserSetting, 0);
        } break;

        default: {
            nRet = FALSE;
        } break;
    }

    return nRet;
}
static void SaveUserSetting(PUSERSETTING pUserSetting)
{
    PDLLDOMESETTINGDATA pData = pUserSetting->pData;

    Fc_strcpy(pUserSetting->pCfg[FILE_CONVERSION_DLL_DEMO_CFG_DEST_PATH], pData->pDestPath);
    
    pUserSetting->nCfgCnt = FILE_CONVERSION_DLL_DEMO_CFG_CNT;
}
