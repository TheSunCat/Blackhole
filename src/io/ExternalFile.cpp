#include "io/ExternalFile.h"

ExternalFile::ExternalFile(QString filePath)
        : m_filePath(filePath), m_file(m_filePath)
{
    // don't open automatically so as to allow Dolphin to work
    m_file.open(QIODevice::ReadOnly);
    m_contents = m_file.readAll();
    m_file.close();
}

void ExternalFile::save()
{
    m_file.open(QIODevice::WriteOnly);
    m_file.write(m_contents);
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
}
