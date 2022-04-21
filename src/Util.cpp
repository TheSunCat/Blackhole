#include "Util.h"

QString Util::absolutePath(const QString& gamePath)
{
    return Blackhole::m_gameDir.path() + '/' + gamePath;
}

bool Util::fileExists(const QString& file)
{
    QFile f(Util::absolutePath(file));
    return f.exists();
}

float Util::lerp(float a, float b, float t)
{
    float distance = b - a;
    return a + (distance * t);
}

glm::vec3 Util::lerp(glm::vec3 a, glm::vec3 b, float t)
{
    return glm::vec3(
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t),
        lerp(a.z, b.z, t)
    );
}
