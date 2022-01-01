#pragma once

#include <cstdint>

class QByteArray;

namespace Yaz0
{
    QByteArray decompress(QByteArray data);
    QByteArray compress(QByteArray data);

    struct Occurrence
    {
        int32_t offset = -1; // TODO uint?
        uint32_t length = 0;
    };

    Occurrence findOccurrence(QByteArray data, uint32_t pos);
};
