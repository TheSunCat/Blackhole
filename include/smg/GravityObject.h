#pragma once

#include "smg/BaseObject.h"

class GravityObject : public BaseObject
{
public:
    GravityObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    GravityObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~GravityObject();
};

