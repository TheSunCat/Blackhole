#include "smg/Zone.h"

#include "Util.h"
#include "io/BcsvFile.h"
#include "smg/ZoneObject.h"
#include "smg/MapPartObject.h"
#include "smg/ChildObject.h"
#include "smg/LevelObject.h"
#include "smg/StartObject.h"
#include "smg/GravityObject.h"
#include "smg/SoundObject.h"
#include "smg/AreaObject.h"
#include "smg/CameraObject.h"
#include "smg/CutsceneObject.h"
#include "smg/PositionObject.h"
#include "smg/DebugObject.h"
#include "smg/ChangeObject.h"

Zone::Zone(Galaxy* parent, const QString& name) : m_galaxy(parent), m_zoneName(name)
{
    if(g_gameType == 1)
        m_zoneFileName = "StageData/" + m_zoneName + ".arc";
    if(g_gameType == 2)
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
    if(g_gameType == 1)
    {
        loadObjects("Placement", "SoundInfo");
        loadObjects("ChildObj", "ChildObjInfo");
    }
    if(g_gameType == 2)
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

        if(file == "StageObjInfo")
        {
            for(BcsvFile::Entry& entry : bcsv.m_entries)
            {
                m_zones[layer.toStdString()].push_back(new ZoneObject(*this, dir, layer, file, entry));
            }
        }
        else
        {
            BaseObject* object = nullptr;

            for(BcsvFile::Entry& entry : bcsv.m_entries)
            {
                if(file == "MapPartsInfo")
                    object = new MapPartObject(*this, dir, layer, file, entry);
                else if(file == "ChildObjInfo")
                    object = new ChildObject(*this, dir, layer, file, entry);
                else if(file == "ObjInfo")
                    object = new LevelObject(*this, dir, layer, file, entry);
                else if(file == "StartInfo")
                    object = new StartObject(*this, dir, layer, file, entry);
                else if(file == "PlanetObjInfo")
                    object = new GravityObject(*this, dir, layer, file, entry);
                else if(file == "SoundInfo")
                    object = new SoundObject(*this, dir, layer, file, entry);
                else if(file == "AreaObjInfo")
                    object = new AreaObject(*this, dir, layer, file, entry);
                else if(file == "CameraCubeInfo")
                    object = new CameraObject(*this, dir, layer, file, entry);
                else if(file == "DemoObjInfo")
                    object = new CutsceneObject(*this, dir, layer, file, entry);
                else if(file == "GeneralPosInfo")
                    object = new PositionObject(*this, dir, layer, file, entry);
                else if(file == "DebugMoveInfo")
                    object = new DebugObject(*this, dir, layer, file, entry);
                else if(file == "ChangeObjInfo")
                    object = new ChangeObject(*this, dir, layer, file, entry);

                m_objects[layer.toStdString()].push_back(object);
            }
        }
    }
}
