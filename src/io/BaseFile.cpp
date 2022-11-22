#include "io/BaseFile.h"

#include <QString>
#include <QTextCodec>
#include <array>

void BaseFile::setBigEndian(bool big)
{
    m_bigEndian = big;
}

uint32_t BaseFile::position() const
{
    return m_curPos;
}

void BaseFile::position(uint32_t newPos)
{
    m_curPos = newPos;
}

void BaseFile::skip(uint32_t count)
{
    m_curPos += count;
}

uint32_t BaseFile::getLength() const
{
    return m_contents.size();
}

void BaseFile::setLength(uint32_t length)
{
    m_contents.resize(length);
}


uint8_t BaseFile::readByte() const
{
    return m_contents[m_curPos++]; // post-increment
}

uint16_t BaseFile::readShort() const
{
    uint16_t ret = 0;

    if(m_bigEndian)
    {
        ret |= readByte() << 8;
        ret |= readByte() << 0;
    }
    else
    {
        ret |= readByte() << 0;
        ret |= readByte() << 8;
    }

    return ret;
}

int16_t BaseFile::readShortS() const
{
    int16_t ret = 0;

    uint8_t insignificantByte = readByte();
    uint8_t significantByte   = readByte();

    if(m_bigEndian)
    {
        uint8_t temp = insignificantByte;
        insignificantByte = significantByte;
        significantByte = temp;
    }

    ret |= significantByte   << 7;
    ret |= insignificantByte << 0;

    if(significantByte >> 7)
        ret *= -1;

    return ret;
}


uint32_t BaseFile::readInt() const
{
    uint32_t ret = 0;

    if(m_bigEndian)
    {
        ret |= readByte() << 24;
        ret |= readByte() << 16;
        ret |= readByte() <<  8;
        ret |= readByte() <<  0;
    }
    else
    {
        ret |= readByte() <<  0;
        ret |= readByte() <<  8;
        ret |= readByte() << 16;
        ret |= readByte() << 24;
    }

    return ret;
}

float BaseFile::readFloat() const
{
    // oh boy
    float ret;

    uint8_t bytes[] = {readByte(), readByte(), readByte(), readByte()};

    // hope this works
    if(!m_bigEndian)
    {
        uint8_t temp = bytes[3];
        bytes[3] = bytes[0];
        bytes[0] = temp;

        temp = bytes[2];
        bytes[2] = bytes[1];
        bytes[1] = temp;
    }

    memcpy(&ret, &bytes, sizeof(float));

    return ret;
}

QString BaseFile::readString(uint32_t length, const char* enc) const
{
    if(strcmp(enc, "ASCII") == 0)
        enc = "UTF-8";

    std::vector<uint8_t> bytes;
    for(int i = 0; i < length || length == 0; i++)
    {
        uint8_t byte = readByte();

        if(length == 0 && byte == 0)
        {
            // TODO will this break on multibyte encs like Shift-JIS?
            break;
        }

        bytes.push_back(byte);


    }

    const QByteArray byteArray = QByteArray::fromRawData((const char*)bytes.data(), bytes.size());

    QTextDecoder* dc = QTextCodec::codecForName(enc)->makeDecoder();
    return dc->toUnicode(byteArray);
}

std::vector<uint8_t> BaseFile::readBytes(uint32_t count) const
{
    std::vector<uint8_t> ret(count);

    for(int i = 0; i < count; i++)
        ret[i] = readByte();

    return ret;
}

void BaseFile::writeByte(uint8_t val)
{
    uint8_t oldVal = m_contents[m_curPos];

    m_contents[m_curPos++] = val;

    if(oldVal != val)
        m_modifiedFlag = true;
}

void BaseFile::writeShort(uint16_t val)
{
    if(m_bigEndian)
    {
        writeByte(val >> 8 & 0xFF);
        writeByte(val      & 0xFF);
    }
    else
    {
        writeByte(val      & 0xFF);
        writeByte(val >> 8 & 0xFF);
    }
}

void BaseFile::writeInt(uint32_t val)
{
    if(m_bigEndian)
    {
        writeByte(val >> 24 & 0xFF);
        writeByte(val >> 16 & 0xFF);
        writeByte(val >> 8  & 0xFF);
        writeByte(val       & 0xFF);
    }
    else
    {
        writeByte(val       & 0xFF);
        writeByte(val >> 8  & 0xFF);
        writeByte(val >> 16 & 0xFF);
        writeByte(val >> 24 & 0xFF);
    }
}

void BaseFile::writeFloat(float val)
{
    union U {
        float val;
        std::array<uint8_t, sizeof(float)> bytes;
    } u;

    u.val = val;

    if(m_bigEndian)
    {
        writeByte(u.bytes[0]);
        writeByte(u.bytes[1]);
        writeByte(u.bytes[2]);
        writeByte(u.bytes[3]);
    }
    else
    {
        writeByte(u.bytes[3]);
        writeByte(u.bytes[2]);
        writeByte(u.bytes[1]);
        writeByte(u.bytes[0]);
    }
}


int BaseFile::writeString(const QString& str, const char* enc)
{
    if(strcmp(enc, "ASCII") == 0)
        enc = "UTF-8";

    QTextEncoder* en = QTextCodec::codecForName(enc)->makeEncoder();
    QByteArray strBytes = en->fromUnicode(str);

    for(auto& byte : strBytes)
        writeByte(byte);

    return strBytes.length();
}

void BaseFile::writeBytes(const std::vector<uint8_t>& bytes)
{
    for(auto& byte : bytes)
        writeByte(byte);
}

const std::vector<uint8_t>& BaseFile::getContents() const
{
    return m_contents;
}

void BaseFile::setContents(const std::vector<uint8_t>& bytes)
{
    m_contents = bytes;
}

std::span<uint8_t> BaseFile::slice(uint32_t start, uint32_t end)
{
    return std::span(m_contents.begin() + start, m_contents.begin() + end);
}
