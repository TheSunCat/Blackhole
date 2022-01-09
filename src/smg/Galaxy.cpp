#include "smg/Galaxy.h"

#include "io/RarcFile.h"
#include "Util.h"

Galaxy::Galaxy(const QString& galaxyName) : m_name(galaxyName)
{
    RarcFile scenario(absolutePath("StageData/" + m_name + '/' + m_name + "Scenario.arc"));
    BcsvFile zonesBcsv(scenario.openFile('/' + m_name + "Scenario/ZoneList.bcsv"));

    for(const BcsvFile::Entry& entry : zonesBcsv.m_entries)
        m_zoneList.push_back(std::get<QString>(entry["ZoneName"]));

    zonesBcsv.close();

    BcsvFile scenarioBcsv(scenario.openFile('/' + m_name + "Scenario/ScenarioData.bcsv"));
    m_scenarioData = scenarioBcsv.m_entries;

    scenarioBcsv.close();
    scenario.close();
}

Zone Galaxy::openZone(const QString& zoneName)
{
    assert(std::find(m_zoneList.begin(), m_zoneList.end(), zoneName) != m_zoneList.end());

    return Zone(this, zoneName);
}
