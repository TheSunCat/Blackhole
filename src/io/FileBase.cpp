#include "io/FileBase.h"

#include <QString>

void FileBase::setBigEndian(bool big)
{
    m_bigEndian = big;
}

uint32_t FileBase::position() const
{
    return m_curPos;
}

void FileBase::position(uint32_t newPos)
{
    m_curPos = newPos;
}

void FileBase::skip(uint32_t count)
{
    m_curPos += count;
}

uint32_t FileBase::getLength() const
{
    return m_contents.size();
}

void FileBase::setLength(uint32_t length)
{
    m_contents.resize(length);
}


uint8_t FileBase::readByte() const
{
    return m_contents[m_curPos++]; // post-increment
}

uint16_t FileBase::readShort() const
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

uint32_t FileBase::readInt() const
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

float FileBase::readFloat() const
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

#include <sstream>
QString FileBase::readString() const
{
    std::wstringstream ret;
    ret.imbue(std::locale(".932"));

    while(m_contents[m_curPos] != '\0')
    {
        ret << readByte(); // TODO fix
    }

    return QString::fromStdWString(ret.str());
}

QByteArray FileBase::readBytes(uint32_t count) const
{
    QByteArray ret(count, 0);

    for(int i = 0; i < count; i++)
        ret[i] = readByte();

    return ret;
}

void FileBase::writeByte(uint8_t val)
{
    uint8_t oldVal = m_contents[m_curPos];

    m_contents[m_curPos++] = val;

    if(oldVal != val)
        m_modifiedFlag = true;
}

void FileBase::writeShort(uint16_t val)
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

void FileBase::writeInt(uint32_t val)
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

void FileBase::writeFloat(float val)
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


int FileBase::writeString(QString str)
{
    // TODO no idea if this is right

    auto str16 = str.toStdU16String();
    const char *begin = reinterpret_cast<char const*>(str16.data());

    QByteArray strBytes(begin, str16.size() * 2);

    // not using QByteArray::replace because we want to set m_modifiedFlag
    for(auto& byte : strBytes)
        writeByte(byte);

    return strBytes.length();
}

void FileBase::writeBytes(QByteArray bytes)
{
    for(auto& byte : bytes)
        writeByte(byte);
}


QByteArray FileBase::getContents() const
{
    return m_contents;
}

void FileBase::setContents(QByteArray bytes)
{
    m_contents.clear(); // TODO do I need to clear?
    m_contents = bytes;
}
