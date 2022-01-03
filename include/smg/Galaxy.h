#pragma once

#include <vector>
#include <QString>

#include "io/BcsvFile.h"

struct Galaxy
{
    Galaxy(QString galaxyName);

    QString name;
    std::vector<QString> zoneList;
    std::vector<BcsvFile::Entry> scenarioData;
};
