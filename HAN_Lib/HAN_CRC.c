#include "HAN_CRC.h"

static uint32_t RefData(uint32_t data, uint8_t len);

uint8_t CRC_SafeSPI(const uint32_t message, uint8_t len, uint8_t poly, uint8_t start_value)
{
    (void)message;
    (void)len;
    (void)poly;
    (void)start_value;
    
    return 0;
}

uint8_t CRC4_7(const void* data, size_t len, uint8_t poly, uint8_t start_value, uint8_t xor_out, int ref_in, int ref_out, int mode)
{
    uint32_t ret32;
    uint16_t poly8 = poly << (16 - mode);
    const uint8_t* buffer = data;

    if (ref_in) { ret32 = RefData(buffer[0], 8) << 8; }
    else { ret32 = buffer[0] << 8; }

    ret32 ^= start_value << (16 - mode);

    for (size_t i = 0; i < len; i++)
    {
        if (i < len - 1)
        {
            if (ref_in) { ret32 += RefData(buffer[i + 1], 8); }
            else { ret32 += buffer[i + 1]; }
        }
        for (size_t j = 0; j < 8; j++)
        {
            ret32 <<= 1;
            if (ret32 & 0x10000)
            {
                ret32 ^= poly8;
            }
        }
    }

    uint8_t ret = (ret32 >> (16 - mode)) & ((1 << mode) - 1);
    if (ref_out) { ret = RefData(ret, mode); }

    return ret ^ xor_out;
}

uint8_t CRC8(const void* data, size_t len, uint8_t poly, uint8_t start_value, uint8_t xor_out, int ref_in, int ref_out)
{
#ifdef HAN_CRC_ENABLE
    uint32_t ret32;
    uint16_t poly8 = poly << 8;
    const uint8_t* buffer = data;

    if (ref_in) { ret32 = RefData(buffer[0], 8) << 8; }
    else { ret32 = buffer[0] << 8; }

    ret32 ^= start_value << 8;

    for (size_t i = 0; i < len; i++)
    {
        if (i < len - 1)
        {
            if (ref_in) { ret32 += RefData(buffer[i + 1], 8); }
            else { ret32 += buffer[i + 1]; }
        }
        for (size_t j = 0; j < 8; j++)
        {
            ret32 <<= 1;
            if (ret32 & 0x10000)
            {
                ret32 ^= poly8;
            }
        }
    }

    uint8_t ret = (ret32 >> 8) & 0xFF;
    if (ref_out) { ret = RefData(ret, 8); }

    return ret ^ xor_out;
#else
    uint8_t ret = start_value;
    const uint8_t* buffer = data;

    for (size_t i = 0; i < len; i++)
    {
        if (ref_in) { ret ^= RefData(buffer[i], 8); }
        else { ret ^= buffer[i]; }

        for (int j = 0; j < 8; j++)
        {
            if (ret & 0x80)
            {
                ret <<= 1;
                ret ^= poly;
            }
            else { ret <<= 1; }
        }
    }

    if (ref_out) { ret = RefData(ret, 8); }

    return ret ^ xor_out;
#endif
}

uint16_t CRC16(const void* data, size_t len, uint16_t poly, uint16_t start_value, uint16_t xor_out, int ref_in, int ref_out)
{
#ifdef HAN_CRC_ENABLE
    uint32_t ret32 = 0;
    uint32_t poly8 = poly << 8;
    const uint8_t* buffer = data;
    
    for (int i = 0; i < 2; i++)
    {
        if (ref_in) { ret32 += RefData(buffer[i], 8); }
        else { ret32 += buffer[i]; }
        ret32 <<= 8;
    }

    ret32 ^= start_value << 8;

    for (size_t i = 0; i < len; i++)
    {
        if (i < len - 2)
        {
            if (ref_in) { ret32 += RefData(buffer[i + 2], 8); }
            else { ret32 += buffer[i + 2]; }
        }
        for (size_t j = 0; j < 8; j++)
        {
            ret32 <<= 1;
            if (ret32 & 0x1000000)
            {
                ret32 ^= poly8;
            }
        }
    }

    uint16_t ret = (ret32 >> 8) & 0xFFFF;
    if (ref_out) { ret = RefData(ret, 16); }

    return ret ^ xor_out;
#else
    uint16_t ret = start_value;
    const uint8_t* buffer = data;

    for (size_t i = 0; i < len; i++)
    {
        if (ref_in) { ret ^= RefData(buffer[i], 8) << 8; }
        else { ret ^= buffer[i] << 8; }

        for (int j = 0; j < 8; j++)
        {
            if (ret & 0x8000)
            {
                ret <<= 1;
                ret ^= poly;
            }
            else { ret <<= 1; }
        }
    }

    if (ref_out) { ret = RefData(ret, 16); }

    return ret ^ xor_out;
#endif
}

uint32_t CRC32(const void* data, size_t len, uint32_t poly, uint32_t start_value, uint32_t xor_out, int ref_in, int ref_out)
{
#ifdef HAN_CRC_ENABLE
    uint32_t ret = 0;
    const uint8_t* buffer = data;
    uint8_t next_byte;
    
    for (int i = 0; i < 4; i++)
    {
        ret <<= 8;
        if (ref_in) { ret += RefData(buffer[i], 8); }
        else { ret += buffer[i]; }
    }

    ret ^= start_value;

    for (size_t i = 0; i < len; i++)
    {
        if (i < len - 4)
        {
            next_byte = buffer[i + 4];
            if (ref_in)
                next_byte = RefData(next_byte, 8);
        }
        else { next_byte = 0; }
        for (size_t j = 0; j < 8; j++)
        {
            if (ret & 0x80000000)
            {
                ret <<= 1;
                if (next_byte & 0x80) { ret++; }
                ret ^= poly;
            }
            else
            {
                ret <<= 1;
                if (next_byte & 0x80) { ret++; }
            }
            next_byte <<= 1;
        }
    }

    if (ref_out) { ret = RefData(ret, 32); }

    return ret ^ xor_out;
#else
    uint32_t ret = start_value;
    const uint8_t* buffer = data;
    for (size_t i = 0; i < len; i++)
    {
        if (ref_in) { ret ^= RefData(buffer[i], 8) << 24; }
        else { ret ^= (buffer[i]) << 24; }
        
        for (int j = 0; j < 8; j++)
        {
            if (ret & 0x80000000)
            {
                ret <<= 1;
                ret ^= poly;
            }
            else
            {
                ret <<= 1;
            }
        }
    }

    if (ref_out) { ret = RefData(ret, 32); }

    return ret ^ xor_out;
#endif
}

static uint32_t RefData(uint32_t data, uint8_t len)
{
#ifdef HAN_CRC_ENABLE
    uint32_t ret = 0;
    for (int i = 0; i < len; i++)
    {
        ret <<= 1;
        if (data & 1) { ret++; }
        data >>= 1;
    }
    return ret;
#else
    data = ((data & 0xFFFF0000) >> 16) | ((data & 0x0000FFFF) << 16);
    data = ((data & 0xFF00FF00) >> 8) | ((data & 0x00FF00FF) << 8);
    data = ((data & 0xF0F0F0F0) >> 4) | ((data & 0x0F0F0F0F) << 4);
    data = ((data & 0xCCCCCCCC) >> 2) | ((data & 0x33333333) << 2);
    data = ((data & 0xAAAAAAAA) >> 1) | ((data & 0x55555555) << 1);

    switch (len) {
        case 32:
            return data;
        case 16:
            return (data >> 16) & 0xFFFF;
        case 8:
            return (data >> 24) & 0xFF;
        case 7:
            return (data >> 25) & 0x7F;
        case 6:
            return (data >> 26) & 0x3F;
        case 5:
            return (data >> 27) & 0x1F;
        case 4:
            return (data >> 28) & 0x0F;
        default:
            return 0;
    }
#endif
}
