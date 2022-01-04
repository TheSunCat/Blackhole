#pragma once

#include "FileBase.h"

class MemoryFile : public FileBase
{
public:
    MemoryFile() = default;

    MemoryFile(QByteArray buffer);

    virtual void save() override;
    virtual void close() override;
};
