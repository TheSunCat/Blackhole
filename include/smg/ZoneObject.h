#pragma once

#include "smg/BaseObject.h"

class ZoneObject : public BaseObject
{
public:
    ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;
};
