#include "Util.h"

QString absolutePath(const QString& gamePath)
{
    return Blackhole::m_gameDir.path() + '/' + gamePath;
}

bool fileExists(const QString& file)
{
    QFile f(absolutePath(file));
    return f.exists();
}
