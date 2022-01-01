#pragma once

#include "FileBase.h"

class MemoryFile : public FileBase
{
public:
    MemoryFile() = default;

    MemoryFile(QByteArray buffer);

    void save() override;
    void close() override;
};
