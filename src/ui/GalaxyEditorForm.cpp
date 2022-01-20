#include "ui/GalaxyEditorForm.h"
#include "ui_GalaxyEditorForm.h"

#include <iostream>
#include <string>

#include "ui/Blackhole.h"
#include "smg/ZoneObject.h"

GalaxyEditorForm::GalaxyEditorForm(QWidget *parent, const QString& galaxyName) :
    QDialog(parent), m_ui(new Ui::GalaxyEditorForm), m_galaxy(galaxyName)
{
    m_ui->setupUi(this);
    setWindowTitle(QString::fromStdString(blackholeName) + " - Editing " + m_galaxy.m_name);
    std::cout << "Created GalaxyEditorForm for " << m_galaxy.m_name.toStdString() << std::endl;


    for(const QString& zoneName : m_galaxy.m_zoneList)
    {
        // load the zone
        Zone zone = m_galaxy.openZone(zoneName);
        m_zones.insert(std::make_pair(zoneName.toStdString(), zone));

        // add all objects from this zone
        for(const auto& [ layer, objectList ] : zone.m_objects)
        {
            for(BaseObject* object : objectList) {
                // TODO do I need maxUniqueID?
                m_objects.push_back(object);

                std::cout << object->m_name.toStdString() << std::endl;
            }
        }

        // TODO add paths from m_paths
    }

    Zone mainZone = m_zones[galaxyName.toStdString()];
    for(int i = 0; i < m_galaxy.m_scenarioData.size(); i++)
    {
        // add subzones of the main zone
        if(mainZone.m_zones.find("common") != mainZone.m_zones.end())
        {
            for(ZoneObject* subZone : mainZone.m_zones["common"])
            {
                QString key = QChar(char(i)) + '/' + subZone->m_name;

                if(m_zoneObjects.find(key.toStdString()) == m_zoneObjects.end())
                    continue; // TODO error duplicate zone

                m_zoneObjects.insert(std::make_pair(key.toStdString(), subZone));
            }
        }

        uint32_t mainLayerMask = m_galaxy.m_scenarioData[i].geti(galaxyName);
        for(char layerID = 0; layerID < 16; layerID++)
        {
            if((mainLayerMask & (1 << layerID)) == 0)
                continue;

            std::string layer = "layer";
            layer += 'a' + layerID;
            if(mainZone.m_zones.find(layer) == mainZone.m_zones.end())
                continue;

            for(ZoneObject* subZone : mainZone.m_zones[layer])
            {
                // jank string operations aa
                QString key; key += char(i);
                key += "/" + subZone->m_name;

                if(m_zoneObjects.find(key.toStdString()) == m_zoneObjects.end())
                    continue; // TODO error duplicate zone

                m_zoneObjects.insert(std::make_pair(key.toStdString(), subZone));
            }
        }
    }
}

GalaxyEditorForm::~GalaxyEditorForm()
{

}
