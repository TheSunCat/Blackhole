#include "rendering/Texture.h"

Texture Texture::fromBTI(const GX::BTI_Texture& bti)
{
    if(bti.data.isNull())
        return decode_Dummy(bti);

    switch(bti.format)
    {
        case GX::TexFormat::RGBA8:
            return decode_RGBA8(bti);
            break;
        default:
            assert(false); // shouldn't be anything but the above (?)
    };
}

Texture Texture::decode_Dummy(const GX::BTI_Texture& bti)
{
    Texture ret;
    ret.width = bti.width;
    ret.height = bti.height;
    ret.pixels = QByteArray(bti.width * bti.height * 4, 0xFF);

    return ret;
}

Texture Texture::decode_RGBA8(const GX::BTI_Texture& bti)
{
    QByteArrayView data = bti.data;

    uint32_t srcOffset = 0;

    constexpr uint32_t bw = 4;
    constexpr uint32_t bh = 4;

    QByteArray pixels(bti.width * bti.height * 4, 0x00);

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
