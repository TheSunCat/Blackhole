#include "smg/Galaxy.h"

#include "io/RarcFile.h"
#include "filetypes/Bcsv.h"

Galaxy::Galaxy(QString galaxyName) : name(galaxyName)
{
    RarcFile scenario("StageData/" + galaxyName + '/' + galaxyName + "Scenario.arc");
    BcsvFile zonesBcsv(scenario.openFile("/" + galaxyName + "Scenario/ZoneList.bcsv"));
}
