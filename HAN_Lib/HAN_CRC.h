#ifndef HAN_CRC_H
#define HAN_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HAN_CRC_ENABLE      // 部分函数有两个版本，注释掉该宏会使用网上找来的代码，使能该宏会使用自己写的代码

// 多项式表
#define POLY_CRC_4_ITU      0x03
#define POLY_CRC_5_EPC      0x09
#define POLY_CRC_5_ITU      0x15
#define POLY_CRC_5_USB      0x05
#define POLY_CRC_6_ITU      0x03
#define POLY_CRC_7_MMC      0x09
#define POLY_CRC_8          0x07
#define POLY_CRC_MAXIM      0x31
#define POLY_CRC_16         0x8005
#define POLY_CRC_16_CCITT   0x1021
#define POLY_CRC_16_DNP     0x3D65
#define POLY_CRC_32         0x04C11DB7
#define POLY_CRC_32C        0x1EDC6F41

uint8_t CRC_SafeSPI(const uint32_t message, uint8_t len, uint8_t poly, uint8_t start_value);
uint8_t CRC4_7(const void* data, size_t len, uint8_t poly, uint8_t start_value, uint8_t xor_out, int ref_in, int ref_out, int mode);
uint8_t CRC8(const void* data, size_t len, uint8_t poly, uint8_t start_value, uint8_t xor_out, int ref_in, int ref_out);
uint16_t CRC16(const void* data, size_t len, uint16_t poly, uint16_t start_value, uint16_t xor_out, int ref_in, int ref_out);
uint32_t CRC32(const void* data, size_t len, uint32_t poly, uint32_t start_value, uint32_t xor_out, int ref_in, int ref_out);

// 常用的 CRC 校验
#define CRC4_ITU(data, len)             CRC4_7(data, len, POLY_CRC_4_ITU, 0, 0, 1, 1, 4)
#define CRC5_EPC(data, len)             CRC4_7(data, len, POLY_CRC_5_EPC, 0X09, 0, 0, 0, 5)
#define CRC5_ITU(data, len)             CRC4_7(data, len, POLY_CRC_5_ITU, 0, 0, 1, 1, 5)
#define CRC5_USB(data, len)             CRC4_7(data, len, POLY_CRC_5_USB, 0x1F, 0x1F, 1, 1, 5)
#define CRC6_ITU(data, len)             CRC4_7(data, len, POLY_CRC_6_ITU, 0, 0, 1, 1, 6)
#define CRC7_MMC(data, len)             CRC4_7(data, len, POLY_CRC_7_MMC, 0, 0, 0, 0, 7)
#define CRC8_STD(data, len)             CRC8(data, len, POLY_CRC_8, 0, 0, 0, 0)
#define CRC8_ITU(data, len)             CRC8(data, len, POLY_CRC_8, 0, 0x55, 0, 0)
#define CRC8_ROHC(data, len)            CRC8(data, len, POLY_CRC_8, 0xFF, 0, 1, 1)
#define CRC8_MAXIM(data, len)           CRC8(data, len, POLY_CRC_MAXIM, 0, 0, 1, 1)
#define CRC16_IBM(data, len)            CRC16(data, len, POLY_CRC_16, 0, 0, 1, 1)
#define CRC16_MAXIM(data, len)          CRC16(data, len, POLY_CRC_16, 0, 0xFFFF, 1, 1)
#define CRC16_USB(data, len)            CRC16(data, len, POLY_CRC_16, 0xFFFF, 0xFFFF, 1, 1)
#define CRC16_MODEBUS(data, len)        CRC16(data, len, POLY_CRC_16, 0xFFFF, 0, 1, 1)
#define CRC16_CCITT(data, len)          CRC16(data, len, POLY_CRC_16_CCITT, 0, 0, 1, 1)
#define CRC16_CCITT_FALSE(data, len)    CRC16(data, len, POLY_CRC_16_CCITT, 0xFFFF, 0, 0, 0)
#define CRC16_X25(data, len)            CRC16(data, len, POLY_CRC_16_CCITT, 0xFFFF, 0xFFFF, 1, 1)
#define CRC16_XMODEM(data, len)         CRC16(data, len, POLY_CRC_16_CCITT, 0, 0, 0, 0)
#define CRC16_DNP(data, len)            CRC16(data, len, POLY_CRC_16_DNP, 0, 0xFFFF, 1, 1)
#define CRC32_STD(data, len)            CRC32(data, len, POLY_CRC_32, 0xFFFFFFFF, 0xFFFFFFFF, 1, 1)
#define CRC32_MPEG2(data, len)          CRC32(data, len, POLY_CRC_32, 0xFFFFFFFF, 0, 0, 0)

#ifdef __cplusplus
}
#endif

#endif
