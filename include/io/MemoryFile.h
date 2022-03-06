#pragma once

#include "BaseFile.h"

class MemoryFile : public BaseFile
{
public:
    MemoryFile() = default;

    MemoryFile(const std::vector< uint8_t >& buffer)
;

    virtual void save() override;
    virtual void close() override;
};
