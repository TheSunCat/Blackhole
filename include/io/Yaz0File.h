#pragma once

#include "MemoryFile.h"
#include "ExternalFile.h"

class ExternalFile;

class Yaz0File : public MemoryFile
{
    ExternalFile m_backend;

public:
    Yaz0File(QString filePath);

    void save() override;
};
