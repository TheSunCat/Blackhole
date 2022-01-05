#include "io/MemoryFile.h"

MemoryFile::MemoryFile(QByteArray buffer)
{
    m_contents = buffer;
}

void MemoryFile::save()
{
    // implemented in subclasses
    assert(false);
}

void MemoryFile::close()
{
    // implemented in subclasses
    //assert(false);
}
