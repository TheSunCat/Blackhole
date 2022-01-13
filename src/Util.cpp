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

float lerp(float a, float b, float t)
{
    float distance = b - a;
    return a + (distance * t);
}

glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t)
{
    return glm::vec3(
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t),
        lerp(a.z, b.z, t)
    );
}
