#include "Util.h"

QString absolutePath(const QString& gamePath)
{
    return Blackhole::m_gameDir.path() + '/' + gamePath;
}
