#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "io/RarcFile.h"

struct Galaxy;
class BaseObject;
class ZoneObject;
class PathObject;

class Zone
{
    Galaxy* m_galaxy;
    RarcFile m_map;

    QString m_zoneFileName;
    QString m_zoneName;

    void loadObjects(const QString& dir, const QString& file);

public:
    Zone() = default;

    Zone(Galaxy* parent, const QString& name);

    std::unordered_map<std::string, std::vector<BaseObject*>> m_objects;
    std::unordered_map<std::string, std::vector<ZoneObject*>> m_zones;
    std::vector<PathObject*> m_paths;
};
