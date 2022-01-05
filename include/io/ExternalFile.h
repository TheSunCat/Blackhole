#pragma once
#include "io/BaseFile.h"

#include <QString>
#include <QFile>

class ExternalFile : public BaseFile
{
    QString m_filePath;

    QFile m_file;
public:
    ExternalFile(const QString& filePath);

    virtual void save() override;
    virtual void close() override;

    virtual uint32_t getLength() const override;
    virtual void setLength(uint32_t length) override;
};
