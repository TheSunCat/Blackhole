#pragma once

#include "smg/BaseObject.h"

class AreaObject : public BaseObject
{
public:
    AreaObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    AreaObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~AreaObject();
};


