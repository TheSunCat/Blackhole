#include "io/ExternalFile.h"

ExternalFile::ExternalFile(const QString& filePath)
        : m_filePath(filePath), m_file(m_filePath)
{
    // don't open automatically so as to allow Dolphin to work
    m_file.open(QIODevice::ReadOnly);
    QByteArray bytes = m_file.readAll();

    m_contents = std::vector<uint8_t>(bytes.begin(), bytes.end());
    m_file.close();
}

void ExternalFile::save()
{
    m_file.open(QIODevice::WriteOnly);
    m_file.write(QByteArray((const char*)m_contents.data(), m_contents.size()));
    m_file.close();
}

void ExternalFile::close()
{
    m_file.close();
}

uint32_t ExternalFile::getLength() const
{
    return m_file.size();
}

void ExternalFile::setLength(uint32_t length)
{
    // TODO this function does not exist in QFile... do we even need it?
    //m_file.setLength(length);
    assert(false);
}
