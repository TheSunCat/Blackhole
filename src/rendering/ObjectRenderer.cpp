#include "rendering/ObjectRenderer.h"

#include "Util.h"
#include "rendering/Texture.h"

#include <iostream>
#include <future>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

ObjectRenderer::ObjectRenderer(BaseObject* obj)
        : m_object(obj), m_translation(obj->m_pos),
          m_rotation(obj->m_rot), m_scale(obj->m_scl),
          m_modelName(obj->m_name)
{
    QString filePath = absolutePath("ObjectData/" + m_modelName + ".arc");

    QFileInfo fileInfo = QFileInfo(filePath);
    if(!fileInfo.exists())
        return; // TODO fallback to cube

    m_rarc = RarcFile(filePath);
    if(m_rarc.fileExists('/' + m_modelName + '/' + m_modelName + ".bdl"))
        m_model = BmdFile(m_rarc.openFile('/' + m_modelName + '/' + m_modelName + ".bdl"));
    else if(m_rarc.fileExists('/' + m_modelName + '/' + m_modelName + ".bmd"))
        m_model = BmdFile(m_rarc.openFile('/' + m_modelName + '/' + m_modelName + ".bmd"));

    std::vector<std::future<void>> futures;
    std::mutex m;

    for(auto& tex : m_model.m_textures)
    {
        futures.push_back(std::async(std::launch::async, [&] {
            Texture decTex = Texture::fromBTI(tex);

            std::lock_guard<std::mutex> lock(m);
            m_textures.push_back(std::move(decTex));
        }));
    }
}
