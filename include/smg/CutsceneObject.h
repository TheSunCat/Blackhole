#pragma once

#include "smg/BaseObject.h"

class CutsceneObject : public BaseObject
{
public:
    CutsceneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    CutsceneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~CutsceneObject();
};
