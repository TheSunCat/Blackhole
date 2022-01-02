#pragma once

#include <vector>
#include <QString>

struct Galaxy
{
    Galaxy(QString galaxyName);

    QString name;
    std::vector<QString> zoneList;
    std::vector<BcsvFile::Entry> scenarioData;
}
