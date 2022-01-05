#include "io/BaseFile.h"

#include <QString>
#include <QTextCodec>

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
    QByteArray bytes;
    for(int i = 0; i < length || length == 0; i++)
    {
        uint8_t byte = readByte();

        if(length == 0 && byte == 0)
        {
            // TODO will this break on multibyte encs like Shift_JIS?
            break;
        }

        bytes.push_back(byte);


    }


    QTextDecoder* dc = QTextCodec::codecForName(enc)->makeDecoder();
    return dc->toUnicode(bytes);
}

QByteArray BaseFile::readBytes(uint32_t count) const
{
    QByteArray ret(count, 0);

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
    QTextEncoder* en = QTextCodec::codecForName(enc)->makeEncoder();
    QByteArray strBytes = en->fromUnicode(str);

    // not using QByteArray::replace because we want to set m_modifiedFlag
    for(auto& byte : strBytes)
        writeByte(byte);

    return strBytes.length();
}

void BaseFile::writeBytes(QByteArray bytes)
{
    for(auto& byte : bytes)
        writeByte(byte);
}

QByteArray BaseFile::getContents() const
{
    return m_contents;
}

void BaseFile::setContents(QByteArray bytes)
{
    m_contents = bytes;
}
