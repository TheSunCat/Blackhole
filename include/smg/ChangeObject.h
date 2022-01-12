#pragma once

#include "smg/BaseObject.h"

class ChangeObject : public BaseObject
{
public:
    ChangeObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    ChangeObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~ChangeObject();
};
