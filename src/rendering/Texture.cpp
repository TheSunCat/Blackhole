#include "rendering/Texture.h"

#include <array>

Texture Texture::fromBTI(const GX::BTI_Texture& bti)
{
    if(bti.data.empty())
        return decode_Dummy(bti);

    switch(bti.format)
    {
        case GX::TexFormat::I4:
            return decode_I4(bti);

        case GX::TexFormat::I8:
            return decode_I8(bti);

        case GX::TexFormat::IA4:
            return decode_IA4(bti);

        case GX::TexFormat::IA8:
            return decode_IA8(bti);

        case GX::TexFormat::RGB565:
            return decode_RGB565(bti);

        case GX::TexFormat::RGB5A3:
            return decode_RGB5A3(bti);

        case GX::TexFormat::RGBA8:
            return decode_RGBA8(bti);

        case GX::TexFormat::CMPR:
            return decode_CMPR(bti);

        default:
            assert(false); // shouldn't be anything but the above (?)
    };
}

Texture Texture::decode_Dummy(const GX::BTI_Texture& bti)
{
    Texture ret;
    ret.width = bti.width;
    ret.height = bti.height;
    ret.pixels = std::vector<uint8_t>(bti.width * bti.height * 4, 0xFF);

    return ret;
}

Texture Texture::decode_I4(const GX::BTI_Texture& bti)
{
    const std::span<const uint8_t>& data = bti.data;
    uint32_t srcOffset = 0;

    return decode_Tiled(bti, 8, 8, [&](std::vector<uint8_t>& pixels, uint32_t& dstOffset) {
        uint8_t ii = data[srcOffset >> 1];
        uint8_t i4 = ii >> ((srcOffset & 1) ? 0 : 4) & 0x0F;
        uint16_t i = expand4to8(i4);

        pixels[dstOffset + 0] = i;
        pixels[dstOffset + 1] = i;
        pixels[dstOffset + 2] = i;
        pixels[dstOffset + 3] = i;

        srcOffset++;
    });
}

Texture Texture::decode_I8(const GX::BTI_Texture& bti)
{
    const std::span<const uint8_t>& data = bti.data;
    uint32_t srcOffset = 0;

    return decode_Tiled(bti, 8, 4, [&](std::vector<uint8_t>& pixels, uint32_t& dstOffset) {
        uint8_t i = data[srcOffset];

        pixels[dstOffset + 0] = i;
        pixels[dstOffset + 1] = i;
        pixels[dstOffset + 2] = i;
        pixels[dstOffset + 3] = i;

        srcOffset++;
    });
}

Texture Texture::decode_IA4(const GX::BTI_Texture& bti)
{
    const std::span<const uint8_t>& data = bti.data;
    uint32_t srcOffset = 0;

    return decode_Tiled(bti, 8, 4, [&](std::vector<uint8_t>& pixels, uint32_t& dstOffset) {
        uint8_t ia = data[srcOffset];
        uint8_t a = expand4to8(ia >> 4);
        uint8_t i = expand4to8(ia & 0x0F);

        pixels[dstOffset + 0] = i;
        pixels[dstOffset + 1] = i;
        pixels[dstOffset + 2] = i;
        pixels[dstOffset + 3] = a;

        srcOffset++;
    });
}

Texture Texture::decode_IA8(const GX::BTI_Texture& bti)
{
    const std::span<const uint8_t>& data = bti.data;
    uint32_t srcOffset = 0;

    return decode_Tiled(bti, 4, 4, [&](std::vector<uint8_t>& pixels, uint32_t& dstOffset) {
        uint8_t a = data[srcOffset];
        uint8_t i = data[srcOffset + 1];

        pixels[dstOffset + 0] = i;
        pixels[dstOffset + 1] = i;
        pixels[dstOffset + 2] = i;
        pixels[dstOffset + 3] = a;

        srcOffset += 2;
    });
}

Texture Texture::decode_RGB565(const GX::BTI_Texture& bti)
{
    const std::span<const uint8_t>& data = bti.data;
    uint32_t srcOffset = 0;

    return decode_Tiled(bti, 4, 4, [&](std::vector<uint8_t>& pixels, uint32_t& dstOffset) {
        uint16_t p = data[srcOffset] << 8 & data[srcOffset + 1]; // TODO little-endian

        pixels[dstOffset + 0] = expand5to8((p >> 11) & 0x1F);
        pixels[dstOffset + 1] = expand6to8((p >> 5) & 0x3F);
        pixels[dstOffset + 2] = expand5to8(p & 0x1F);
        pixels[dstOffset + 3] = 0xFF;

        srcOffset += 2;
    });
}

Texture Texture::decode_RGB5A3(const GX::BTI_Texture& bti)
{
    const std::span<const uint8_t>& data = bti.data;
    uint32_t srcOffset = 0;

    return decode_Tiled(bti, 4, 4, [&](std::vector<uint8_t>& pixels, uint32_t& dstOffset) {
        uint16_t p = data[srcOffset] << 8 & data[srcOffset + 1]; // TODO little-endian

        if (p & 0x8000) {
            // RGB5
            pixels[dstOffset + 0] = expand5to8((p >> 10) & 0x1F);
            pixels[dstOffset + 1] = expand5to8((p >> 5) & 0x1F);
            pixels[dstOffset + 2] = expand5to8(p & 0x1F);
            pixels[dstOffset + 3] = 0xFF;
        } else {
            // A3RGB4
            pixels[dstOffset + 0] = expand4to8((p >> 8) & 0x0F);
            pixels[dstOffset + 1] = expand4to8((p >> 4) & 0x0F);
            pixels[dstOffset + 2] = expand4to8(p & 0x0F);
            pixels[dstOffset + 3] = expand3to8(p >> 12);
        }

        srcOffset += 2;
    });
}

Texture Texture::decode_RGBA8(const GX::BTI_Texture& bti)
{
   const std::span<const uint8_t>& data = bti.data;

    uint32_t srcOffset = 0;

    constexpr uint32_t bw = 4;
    constexpr uint32_t bh = 4;

    std::vector<uint8_t> pixels(bti.width * bti.height * 4, 0x00);

    for(uint32_t yy = 0; yy < bti.height; yy += bh)
    {
        for(uint32_t xx = 0; xx < bti.height; xx += bw)
        {
            for(uint32_t y = 0; y < bh; y++)
            {
                for(uint32_t x = 0; x < bw; x++)
                {
                    uint32_t dstPixel = (bti.width * (yy + y)) + xx + x;
                    uint32_t dstOffset = dstPixel * 4;

                    pixels[dstOffset + 3] = data[srcOffset];
                    pixels[dstOffset]     = data[srcOffset + 1];
                    srcOffset += 2;
                }
            }

            for(uint32_t y = 0; y < bh; y++)
            {
                for(uint32_t x = 0; x < bw; x++)
                {
                    uint32_t dstPixel = (bti.width * (yy + y)) + xx + x;
                    uint32_t dstOffset = dstPixel * 4;
                    pixels[dstOffset + 1] = data[srcOffset];
                    pixels[dstOffset + 2] = data[srcOffset + 1];
                    srcOffset += 2;
                }
            }
        }
    }

    return { "RGBA", bti.width, bti.height, pixels };
}

Texture Texture::decode_CMPR(const GX::BTI_Texture& bti)
{
    // GX's CMPR format is S3TC but using GX's tiled addressing.
    std::vector<uint8_t> pixelsArray(bti.width * bti.height * 4, 0x00);
    uint8_t* pixels = (uint8_t*)pixelsArray.data();

    uint8_t* data = (uint8_t*)bti.data.data();

    std::array<uint8_t, 16> colorTable;

    // CMPR swizzles macroblocks to be in a 2x2 grid of UL, UR, BL, BR.

    uint32_t srcOffset = 0;
    for (uint32_t yy = 0; yy < bti.height; yy += 8) {
        for (uint32_t xx = 0; xx < bti.width; xx += 8) {
            for (uint32_t yb = 0; yb < 8; yb += 4) {
                for (uint32_t xb = 0; xb < 8; xb += 4) {
                    // CMPR difference: Big-endian color1/2
                    uint16_t color1 = (data[srcOffset + 0x0] << 8) | (data[srcOffset + 0x1]); // TODO little endian
                    uint16_t color2 = (data[srcOffset + 0x2] << 8) | (data[srcOffset + 0x3]);

                    // Fill in first two colors in color table.
                    colorTable[0] = expand5to8((color1 >> 11) & 0x1F);
                    colorTable[1] = expand6to8((color1 >> 5) & 0x3F);
                    colorTable[2] = expand5to8(color1 & 0x1F);
                    colorTable[3] = 0xFF;

                    colorTable[4] = expand5to8((color2 >> 11) & 0x1F);
                    colorTable[5] = expand6to8((color2 >> 5) & 0x3F);
                    colorTable[6] = expand5to8(color2 & 0x1F);
                    colorTable[7] = 0xFF;


                    if (color1 > color2) {
                        // Predict gradients.
                        colorTable[8]  = s3tcblend(colorTable[4], colorTable[0]);
                        colorTable[9]  = s3tcblend(colorTable[5], colorTable[1]);
                        colorTable[10] = s3tcblend(colorTable[6], colorTable[2]);
                        colorTable[11] = 0xFF;

                        colorTable[12] = s3tcblend(colorTable[0], colorTable[4]);
                        colorTable[13] = s3tcblend(colorTable[1], colorTable[5]);
                        colorTable[14] = s3tcblend(colorTable[2], colorTable[6]);
                        colorTable[15] = 0xFF;
                    } else {
                        colorTable[8]  = halfblend(colorTable[0], colorTable[4]);
                        colorTable[9]  = halfblend(colorTable[1], colorTable[5]);
                        colorTable[10] = halfblend(colorTable[2], colorTable[6]);
                        colorTable[11] = 0xFF;

                        // CMPR difference: GX fills with an alpha 0 midway point here.
                        colorTable[12] = colorTable[8];
                        colorTable[13] = colorTable[9];
                        colorTable[14] = colorTable[10];
                        colorTable[15] = 0x00;
                    }

                    for (uint32_t y = 0; y < 4; y++) {
                        uint8_t bits = data[srcOffset + 0x04 + y];

                        for (uint32_t x = 0; x < 4; x++) {
                            uint32_t dstPx = (yy + yb + y) * bti.width + xx + xb + x;
                            uint32_t dstOffs = dstPx * 4;
                            uint32_t colorIdx = (bits >> 6) & 0b11;

                            assert(colorIdx >= 0 && colorIdx < 16);

                            pixels[dstOffs + 0] = colorTable[colorIdx * 4 + 0];
                            pixels[dstOffs + 1] = colorTable[colorIdx * 4 + 1];
                            pixels[dstOffs + 2] = colorTable[colorIdx * 4 + 2];
                            pixels[dstOffs + 3] = colorTable[colorIdx * 4 + 3];

                            bits <<= 2;
                        }
                    }

                    srcOffset += 8;
                }
            }
        }
    }

    return { "RGBA", bti.width, bti.height, pixelsArray };
}

Texture Texture::decode_Tiled(const GX::BTI_Texture& bti, uint32_t bw, uint32_t bh, Texture::DecoderFunction decoderFunc)
{
    std::vector<uint8_t> pixels(bti.width * bti.height * 4, 0x00);

    for (uint32_t yy = 0; yy < bti.height; yy += bh) {
        for (uint32_t xx = 0; xx < bti.width; xx += bw) {

            for (uint32_t y = 0; y < bh; y++) {
                for (uint32_t x = 0; x < bw; x++) {

                    uint32_t dstPixel = (bti.width * (yy + y)) + xx + x;
                    uint32_t dstOffset = dstPixel * 4;

                    decoderFunc(pixels, dstOffset);
                }
            }
        }
    }
    return { "RGBA", bti.width, bti.height, pixels };
}


uint16_t Texture::expand3to8(uint16_t n) {
    return (n << (8 - 3)) | (n << (8 - 6)) | (n >> (9 - 8));
}

uint16_t Texture::expand4to8(uint16_t n) {
    return (n << 4) | n;
}

uint16_t Texture::expand5to8(uint16_t n) {
    return (n << (8 - 5)) | (n >> (10 - 8));
}

uint16_t Texture::expand6to8(uint16_t n) {
    return (n << (8 - 6)) | (n >> (12 - 8));
}

// GX uses a HW approximation of 3/8 + 5/8 instead of 1/3 + 2/3.
uint8_t Texture::s3tcblend(uint16_t a, uint16_t b) {
    // return (a*3 + b*5) / 8;
    return (((a << 1) + a) + ((b << 2) + b)) >> 3;
}

uint8_t Texture::halfblend(uint16_t a, uint16_t b)
{
    return (a + b) >> 1;
}
