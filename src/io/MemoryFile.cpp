#include "io/MemoryFile.h"

MemoryFile::MemoryFile(const std::vector<uint8_t>& buffer)
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
}
