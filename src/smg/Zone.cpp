#include "smg/Zone.h"

#include "Util.h"
#include "io/BcsvFile.h"

Zone::Zone(Galaxy* parent, const QString& name) : m_galaxy(parent), m_zoneName(name)
{
    if(Blackhole::m_gameType == 1)
        m_zoneFileName = "StageData/" + m_zoneName + ".arc";
    if(Blackhole::m_gameType == 2)
        m_zoneFileName = "StageData/" + m_zoneName + '/' + m_zoneName + "Map.arc";

    m_map = RarcFile(absolutePath(m_zoneFileName));
    loadObjects("Placement", "StageObjInfo");
}

void Zone::loadObjects(const QString& dir, const QString& file)
{
    QStringList layers = m_map.getSubDirectories("/Stage/Jmp/" + dir);

    for(const QString& layer : layers)
    {
        QString filePath = dir + '/' + layer + '/' + file;

        QStringList stuff = filePath.split('/');

        if(m_objects.find(layer.toStdString()) == m_objects.end())
            m_objects.insert(std::make_pair(layer.toStdString(), std::vector<BaseObject*>()));

        if(m_zoneObjects.find(layer.toStdString()) == m_zoneObjects.end())
            m_zoneObjects.insert(std::make_pair(layer.toStdString(), std::vector<ZoneObject*>()));

        BcsvFile bcsv(m_map.openFile("/Stage/Jmp/" + filePath));

    }
}
