#pragma once

#include "smg/BaseObject.h"

class MapPartObject : public BaseObject
{
public:
    MapPartObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    MapPartObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~MapPartObject();
};
