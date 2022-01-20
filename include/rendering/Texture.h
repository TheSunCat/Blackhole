#pragma once

#include "rendering/GX.h"

class Texture
{
public:
    QString type = "RGBA";
    uint16_t width, height;
    QByteArray pixels;

    static Texture fromBTI(const GX::BTI_Texture& bti);

private:
    static Texture decode_Dummy(const GX::BTI_Texture& bti);
    static Texture decode_RGBA8(const GX::BTI_Texture& bti);
};
