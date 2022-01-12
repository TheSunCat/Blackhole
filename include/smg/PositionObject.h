#pragma once

#include "smg/BaseObject.h"

class PositionObject : public BaseObject
{
public:
    PositionObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    PositionObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~PositionObject();
};
