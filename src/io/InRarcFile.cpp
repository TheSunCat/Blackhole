#include "io/InRarcFile.h"

InRarcFile::InRarcFile(RarcFile* fs, const QString& fullName)
        : MemoryFile(fs->getFileContents(fullName)), m_fs(fs), m_fullName(fullName)
{

}

void InRarcFile::save()
{
    m_fs->reinsertFile(*this); // jank C++?
}
