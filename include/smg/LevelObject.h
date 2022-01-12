#pragma once

#include "smg/BaseObject.h"

class LevelObject : public BaseObject
{
public:
    LevelObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    LevelObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~LevelObject();
};
