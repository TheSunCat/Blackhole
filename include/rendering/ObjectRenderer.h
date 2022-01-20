#pragma once

#include "smg/BaseObject.h"
#include "io/BmdFile.h"

class ObjectRenderer
{
    BmdFile m_model;
    RarcFile m_rarc;

    BaseObject* m_object;

    glm::vec3& m_translation;
    glm::vec3& m_rotation;
    glm::vec3& m_scale;

    QString m_modelName;
public:
    ObjectRenderer(BaseObject* obj);
};
