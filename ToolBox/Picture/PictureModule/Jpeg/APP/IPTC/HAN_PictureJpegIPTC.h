/* IPTC 解码工具
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * IPTC 格式有如下两种：
 * 标准格式：
 *      [1Byte 固定标记][1Byte 记录编号][1Byte 数据集编号][2Byte 数据长度 n ≤ 32767][nByte 数据]
 * 扩展格式：
 *      [1Byte 固定标记][1Byte 记录编号][1Byte 数据集编号][2Byte 数据长度的长度 m | 0x8000][mByte 数据长度]
 * 固定标记：
 *      固定为 0x1C
 * 记录编号：
 *      参考标准《IPTC-NAA Information Interchange Model Version No. 4.1》
 * 数据集编号：
 *      参考标准《IPTC-NAA Information Interchange Model Version No. 4.1》
 * 数据长度：
 *      标准格式和扩展格式的主要区别就在于数据长度的格式不同
 *      如果数据长度小于等于 32767（即 0x7FFF），那么采用标准格式
 *      如果数据长度大于 32767，采用扩展格式
 *      在解码时，如果检测到第 4 个字节的最高位是 0，那么从该字节开始的 2 个字节就是数据长度
 *      如果检测到第 4 个字节的最高位是 1，那么先从该字节开始读取 2 个字节，将最高位的 1 去掉，获取数据长度的长度 m，再读取 m 个字节，得到真正的数据长度
 */
#ifndef HAN_PICTURE_JPEG_IPTC_H
#define HAN_PICTURE_JPEG_IPTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_PictureJpegDef.h"

typedef struct tagPICTUREJPEGIPTCDATA {
    uint8_t                 nRecordId;
    uint8_t                 nDataSetId;
    const uint8_t*          pData;
    HANSIZE                 nLen;
} PICTUREJPEGIPTCDATA, * PPICTUREJPEGIPTCDATA;
typedef const PICTUREJPEGIPTCDATA* PCPICTUREJPEGIPTCDATA;

typedef struct tagPICTUREJPEGIPTCDECODEDATA {
    HANPSTR                 pName;
    HANSIZE                 nNameSize;
    HANPSTR                 pValue;
    HANSIZE                 nValueSize;
    UINT                    cCodePage;
    PCPICTUREJPEGIPTCDATA   pIPTCData;
} PICTUREJPEGIPTCDECODEDATA, * PPICTUREJPEGIPTCDECODEDATA;

BOOL PictureJpegDecodeIPTC(PPICTUREJPEGIPTCDECODEDATA pDecodeIPTC);

HANSIZE PictureJpegReadIPTC(const uint8_t* pData, HANSIZE nLen, uint8_t nTagMarker, PPICTUREJPEGIPTCDATA pIPTCData);


#ifdef __cplusplus
}
#endif

#endif
