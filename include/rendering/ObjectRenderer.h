#pragma once

#include "smg/BaseObject.h"
#include "io/BmdFile.h"
#include "rendering/Texture.h"

#include <vector>
#include <future>

class ObjectRenderer
{
    BmdFile m_model;
    RarcFile m_rarc;

    BaseObject* m_object;
    std::vector<Texture> m_textures;

    glm::vec3& m_translation;
    glm::vec3& m_rotation;
    glm::vec3& m_scale;

    QString m_modelName;

    void decodeTexture(GX::BTI_Texture& tex);
public:
    ObjectRenderer(BaseObject* obj);
};
