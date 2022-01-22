#include "rendering/ObjectRenderer.h"

#include "Util.h"
#include "rendering/Texture.h"

#include <iostream>

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


    for(auto& tex : m_model.m_textures)
    {
        std::cout << "Dumping texture " << tex.name.toStdString() << ". Format: " << int(tex.format) << std::endl;

        Texture decTex = Texture::fromBTI(tex);

        std::string fileName = "/home/sunny/Documents/Blackhole/test/"; fileName += m_modelName.toStdString();
        fileName += '_'; fileName += tex.name.toStdString(); fileName += ".png";

        stbi_write_png(fileName.c_str(), tex.width, tex.height, 4, decTex.pixels, tex.width * 4);
    }
}
