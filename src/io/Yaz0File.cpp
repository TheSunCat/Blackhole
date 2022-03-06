#include "io/Yaz0File.h"

#include "io/ExternalFile.h"

#include <vector>

Yaz0File::Yaz0File(QString filePath)
        : MemoryFile(), m_backend(ExternalFile(filePath))
{
    m_contents = decompress(m_backend.getContents());
}

void Yaz0File::save()
{
    std::vector<uint8_t> compressedBytes = compress(m_contents);

    m_backend.setContents(compressedBytes);
    m_backend.save();
    // TODO release storage here maybe
}

std::vector<uint8_t> Yaz0File::decompress(const std::vector<uint8_t>& data)
{
    if(!(data[0] == 'Y' && data[1] == 'a' && data[2] == 'z' && data[3] == '0'))
        return data; // TODO error message?

    uint32_t fullSize = ((data[4] & 0xFF) << 24) | ((data[5] & 0xFF) << 16) | ((data[6] & 0xFF) << 8) | (data[7] & 0xFF);
    std::vector<uint8_t> ret(fullSize);

    int inPos = 16, outPos = 0;
    while(outPos < fullSize)
    {
        uint8_t block = data[inPos++];

        for(int i = 0; i < 8; i++)
        {
            if((block & 0x80) != 0) // copy one plain byte
            {
                ret[outPos++] = data[inPos++];
            }
            else
            {
                // copy N compressed bytes
                uint8_t b1 = data[inPos++];
                uint8_t b2 = data[inPos++];

                uint16_t dist = ((b1 & 0x0F) << 8) | (b2 & 0xFF);
                uint32_t copySrc = outPos - (dist + 1);

                uint32_t nbytes = (b1 & 0xFF) >> 4;

                if(nbytes == 0)
                    nbytes = (data[inPos++] & 0xFF) + 0x12;
                else
                    nbytes += 2;

                for(int j = 0; j < nbytes; j++)
                    ret[outPos++] = ret[copySrc++];
            }

            block <<= 1;
            if(outPos >= fullSize || inPos >= data.size())
                break;
        }
    }

    return ret;
}

std::vector<uint8_t> Yaz0File::compress(const std::vector<uint8_t>& data)
{
    size_t compressedSize = 16 + data.size() + (data.size() / 8);

    std::vector<uint8_t> ret(compressedSize);

    ret[0] = 'Y'; ret[1] = 'a'; ret[2] = 'z'; ret[3] = '0';

    uint32_t fullSize = data.size();
    ret[4] = (fullSize >> 24) & 0xFF;
    ret[5] = (fullSize >> 16) & 0xFF;
    ret[6] = (fullSize >> 8 ) & 0xFF;
    ret[7] = (fullSize      ) & 0xFF;

    int inPos = 0, outPos = 0;
    Occurrence occ;

    while(inPos < fullSize)
    {
        uint32_t dataStart = outPos + 1;
        uint8_t block = 0;

        for(int i = 0; i < 8; i++)
        {
            block <<= 1;

            if(inPos < data.size())
            {
                if(occ.offset == -2) // TODO -2?
                    occ = findOccurrence(data, inPos);

                Occurrence next = findOccurrence(data, inPos + 1);
                if(next.length > occ.length + 1)
                    occ.offset = -1;

                if(occ.offset != -1)
                {
                    uint32_t disp = (inPos - occ.offset) - 1;
                    assert(disp <= 4095); // TODO throw RuntimeException DISP OUT OF RANGE!

                    if(occ.length > 17)
                    {
                        if(occ.length > 273) occ.length = 273; // TODO use clamp

                        ret[dataStart++] = (disp >> 8)         & 0xFF;
                        ret[dataStart++] = disp                & 0xFF;
                        ret[dataStart++] = (occ.length - 18) & 0xFF;
                    }
                    else
                    {
                        ret[dataStart++] = (((occ.length - 2) << 4) | (disp >> 8)) & 0xFF;
                        ret[dataStart++] = disp & 0xFF;
                    }

                    inPos += occ.length;
                    occ.offset = -2;
                }
                else
                {
                    ret[dataStart++] = data[inPos++];
                    block |= 0x01;
                }

                if(occ.offset != -2)
                {
                    occ = next;
                }
            }
        }

        ret[outPos] = block;
        outPos = dataStart;
    }

    return ret;
}

Yaz0File::Occurrence Yaz0File::findOccurrence(const std::vector<uint8_t>& data, uint32_t pos)
{
    Occurrence ret;

    if (pos >= data.size() - 2)
        return ret;

    std::vector<Occurrence> occurrences;

    uint32_t len;
    uint32_t start = (pos > 4096) ? pos - 4096 : 0;
    for (int32_t i = start; i < pos; i++)
    {
        if (i >= data.size() - 2)
            break;

        if (data[i] != data[pos] || data[i + 1] != data[pos + 1] || data[i + 2] != data[pos + 2])
            continue;

        len = 3;
        while ((i + len < data.size()) && (pos + len < data.size()) && (data[i + len] == data[pos + len]))
            len++;

        occurrences.emplace_back(Occurrence{i, len});
    }

    for (Occurrence occ : occurrences)
    {
        if (occ.length > ret.length)
        {
            ret.offset = occ.offset;
            ret.length = occ.length;
        }
    }

    return ret;
}
