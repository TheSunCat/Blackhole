#pragma once

#include <vector>
#include <QString>

#include "io/BcsvFile.h"

struct Galaxy
{
    Galaxy(QString galaxyName);

    QString m_name;
    std::vector<QString> m_zoneList;
    std::vector<BcsvFile::Entry> m_scenarioData;
};
