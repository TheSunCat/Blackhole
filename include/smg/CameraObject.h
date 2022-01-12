#pragma once

#include "smg/BaseObject.h"

class CameraObject : public BaseObject
{
public:
    CameraObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    CameraObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos);

    virtual int save() override;

    virtual ~CameraObject();
};
