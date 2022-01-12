#pragma once

#include "smg/BaseObject.h"

class StartObject : public BaseObject
{
public:
    StartObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    StartObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~StartObject();
};
