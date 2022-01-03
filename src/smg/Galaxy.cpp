#include "smg/Galaxy.h"

#include "io/RarcFile.h"

Galaxy::Galaxy(QString galaxyName) : name(galaxyName)
{
    RarcFile scenario("StageData/" + name + '/' + name + "Scenario.arc");
    BcsvFile zonesBcsv(scenario.openFile('/' + name + "Scenario/ZoneList.bcsv"));

    for(const BcsvFile::Entry& entry : zonesBcsv.entries)
        zoneList.push_back(std::get<QString>(entry["ZoneName"]));

    zonesBcsv.close();

    BcsvFile scenarioBcsv(scenario.openFile('/' + name + "Scenario/ScenarioData.bcsv"));
    scenarioData = scenarioBcsv.entries;

    scenarioBcsv.close();
    scenario.close();
}
