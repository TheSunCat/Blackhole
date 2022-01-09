#pragma once

#include <vector>
#include <QString>

#include "smg/Zone.h"
#include "io/BcsvFile.h"

struct Galaxy
{
    Galaxy(const QString& galaxyName);

    Zone openZone(const QString& zoneName);

    QString m_name;
    std::vector<QString> m_zoneList;
    std::vector<BcsvFile::Entry> m_scenarioData;
};
