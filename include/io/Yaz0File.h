#pragma once

#include "MemoryFile.h"
#include "ExternalFile.h"

#include <vector>

class ExternalFile;

class Yaz0File : public MemoryFile
{
    ExternalFile m_backend;

    struct Occurrence
    {
        int32_t offset = -1; // TODO uint?
        uint32_t length = 0;
    };

    static Occurrence findOccurrence(const std::vector<uint8_t>& data, uint32_t pos);

public:
    Yaz0File(QString filePath);

    void save() override;

    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
};
