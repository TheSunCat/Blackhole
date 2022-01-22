#pragma once

#include "rendering/GX.h"

#include <functional>

class Texture
{
public:
    QString type = "RGBA";
    uint16_t width, height;
    QByteArray pixels;

    static Texture fromBTI(const GX::BTI_Texture& bti);

private:
    static Texture decode_Dummy(const GX::BTI_Texture& bti);

    static Texture decode_I4(const GX::BTI_Texture& bti);       // 0x0
    static Texture decode_I8(const GX::BTI_Texture& bti);       // 0x1
    static Texture decode_IA4(const GX::BTI_Texture& bti);       // 0x2
    static Texture decode_IA8(const GX::BTI_Texture& bti);      // 0x3
    static Texture decode_RGB565(const GX::BTI_Texture& bti);   // 0x4
    static Texture decode_RGB5A3(const GX::BTI_Texture& bti);   // 0x5
    static Texture decode_RGBA8(const GX::BTI_Texture& bti);    // 0x6
    static Texture decode_CMPR (const GX::BTI_Texture& bti);    // 0xE

    // helper functions
    typedef std::function<void(QByteArray& pixels, uint32_t& dstOffset)> DecoderFunction;

    static Texture decode_Tiled(const GX::BTI_Texture& bti, uint32_t bw, uint32_t bh, DecoderFunction decoderFunc);

    static uint16_t expand3to8(uint16_t n);
    static uint16_t expand4to8(uint16_t n);
    static uint16_t expand5to8(uint16_t n);
    static uint16_t expand6to8(uint16_t n);

    static uint8_t s3tcblend(uint16_t a, uint16_t b);
    static uint8_t halfblend(uint16_t a, uint16_t b);
};
