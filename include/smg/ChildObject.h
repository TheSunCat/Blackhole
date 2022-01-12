#pragma once

#include "smg/BaseObject.h"

class ChildObject : public BaseObject
{
public:
    ChildObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    ChildObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~ChildObject();
};

