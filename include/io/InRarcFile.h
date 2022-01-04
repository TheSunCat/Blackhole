#pragma once

#include "io/MemoryFile.h"
#include "io/RarcFile.h"

#include <QString>

class InRarcFile : public MemoryFile
{
    RarcFile* m_fs;

public:
    QString m_fullName;

    InRarcFile(RarcFile* fs, const QString& fullName);

    void save() override;
};
