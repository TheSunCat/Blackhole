#pragma once

#include "MemoryFile.h"
#include "ExternalFile.h"

#include <QByteArray>

class ExternalFile;

class Yaz0File : public MemoryFile
{
    ExternalFile m_backend;

    struct Occurrence
    {
        int32_t offset = -1; // TODO uint?
        uint32_t length = 0;
    };

    static Occurrence findOccurrence(QByteArray data, uint32_t pos);

public:
    Yaz0File(QString filePath);

    void save() override;

    static QByteArray decompress(QByteArray data);
    static QByteArray compress(QByteArray data);
};
