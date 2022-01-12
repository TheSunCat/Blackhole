#include "smg/Zone.h"

#include "Util.h"
#include "io/BcsvFile.h"
#include "smg/ZoneObject.h"

Zone::Zone(Galaxy* parent, const QString& name) : m_galaxy(parent), m_zoneName(name)
{
    if(Blackhole::m_gameType == 1)
        m_zoneFileName = "StageData/" + m_zoneName + ".arc";
    if(Blackhole::m_gameType == 2)
        m_zoneFileName = "StageData/" + m_zoneName + '/' + m_zoneName + "Map.arc";

    m_map = RarcFile(absolutePath(m_zoneFileName));
    loadObjects("Placement", "StageObjInfo");
    loadObjects("MapParts", "MapPartsInfo");
    loadObjects("Placement", "ObjInfo");
    loadObjects("Start", "StartInfo");
    loadObjects("Placement", "PlanetObjInfo");
    loadObjects("Placement", "AreaObjInfo");
    loadObjects("Placement", "CameraCubeInfo");
    loadObjects("Placement", "DemoObjInfo");
    loadObjects("GeneralPos", "GeneralPosInfo");
    loadObjects("Debug", "DebugMoveInfo");
    if(Blackhole::m_gameType == 1)
    {
        loadObjects("Placement", "SoundInfo");
        loadObjects("ChildObj", "ChildObjInfo");
    }
    else if(Blackhole::m_gameType == 2)
    {
        loadObjects("Placement", "ChangeObjInfo");
    }

    // TODO loadPaths()
}

void Zone::loadObjects(const QString& dir, const QString& file)
{
    QStringList layers = m_map.getSubDirectories("/Stage/Jmp/" + dir);

    for(const QString& layer : layers)
    {
        QString filePath = dir + '/' + layer + '/' + file;

        if(m_objects.find(layer.toStdString()) == m_objects.end())
            m_objects.insert(std::make_pair(layer.toStdString(), std::vector<BaseObject*>()));

        if(m_zones.find(layer.toStdString()) == m_zones.end())
            m_zones.insert(std::make_pair(layer.toStdString(), std::vector<ZoneObject*>()));

        BcsvFile bcsv(m_map.openFile("/Stage/Jmp/" + filePath));

        // TODO categorize objects
        for(BcsvFile::Entry& entry : bcsv.m_entries)
        {
            m_objects[layer.toStdString()].push_back(new ZoneObject(*this, dir, layer, file, entry));
        }
    }
}
