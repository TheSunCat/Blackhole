#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "smg/Galaxy.h"
#include "io/RarcFile.h"

class BaseObject;
class ZoneObject;
class PathObject;

class Zone
{
    Galaxy* m_galaxy;
    RarcFile m_map;

    QString m_zoneFileName;
    QString m_zoneName;

    std::unordered_map<std::string, std::vector<BaseObject*>> m_objects;
    std::unordered_map<std::string, std::vector<ZoneObject*>> m_zoneObjects;
    std::vector<PathObject*> m_paths;

    void loadObjects(const QString& dir, const QString& file);

public:
    Zone() = default;

    Zone(Galaxy* parent, const QString& name);
};
