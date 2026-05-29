#ifndef HAN_PICTURE_ICC_PROFILE_H
#define HAN_PICTURE_ICC_PROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct tagICCPROFILEHEADER {
    uint32_t            nFileSize;
    uint8_t             pCMMType[4];
    uint32_t            nVersion;
    uint8_t             pClass[4];
    uint8_t             pColorSpace[4];
    uint8_t             pPCS[4];
    struct {
        uint16_t        nYear;
        uint16_t        nMonth;
        uint16_t        nDay;
        uint16_t        nHour;
        uint16_t        nMinute;
        uint16_t        nSecond;
    } dateTime;
    uint8_t             pSignature[4];
    uint8_t             pPrimaryPlatform[4];
    uint32_t            cFlags;
    uint8_t             pDeviceManufacturer[4];
    uint8_t             pDeviceModel[4];
    uint32_t            pDeviceAttributes[2];
    uint32_t            cRenderingIntent;
    uint32_t            pPCSIlluminant[3];
    uint8_t             pCreator[4];
    uint32_t            nID[4];
} ICCPROFILEHEADER, * PICCPROFILEHEADER;

typedef struct tagICCPROFILETAG {
    uint8_t             pTagId[4];
    uint32_t            nLen;
    const uint8_t*      pData;
} ICCPROFILETAG;

typedef struct tagICCPROFILEDATA {
    ICCPROFILEHEADER    pfHeader;
    uint32_t            nTagNum;
    ICCPROFILETAG       pTag[];
} ICCPROFILEDATA, * PICCPROFILEDATA;

size_t DecodeICCProfileHeader(const uint8_t* pData, size_t nLen, PICCPROFILEHEADER pHeader);

void DecodeICCProfileData(const uint8_t* pData, size_t nLen, PICCPROFILEDATA pICCData);

#ifdef __cplusplus
}
#endif

#endif
