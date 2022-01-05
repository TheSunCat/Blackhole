#pragma once

#include "BaseFile.h"

class MemoryFile : public BaseFile
{
public:
    MemoryFile() = default;

    MemoryFile(QByteArray buffer);

    virtual void save() override;
    virtual void close() override;
};
