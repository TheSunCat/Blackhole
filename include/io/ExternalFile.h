#pragma once
#include "io/FileBase.h"

#include <QString>
#include <QFile>

class ExternalFile : public FileBase
{
    QString m_filePath;

    QFile m_file;
public:
    ExternalFile(QString filePath);

    void save() override;
    void close() override;

    uint32_t getLength() const override;
    void setLength(uint32_t length) override;
};
