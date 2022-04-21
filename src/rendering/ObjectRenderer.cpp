#include "rendering/ObjectRenderer.h"

#include "Util.h"
#include "rendering/Texture.h"

#include <iostream>
#include <future>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb_image_write.h"
#include "rendering/GalaxyRenderer.h"

ObjectRenderer::ObjectRenderer(BaseObject* obj)
        : m_object(obj), m_translation(obj->m_pos),
          m_rotation(obj->m_rot), m_scale(obj->m_scl),
          m_modelName(obj->m_name)
{
    QString filePath = Util::absolutePath("ObjectData/" + m_modelName + ".arc");

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

void ObjectRenderer::initGL()
{
    auto gl = GalaxyRenderer::gl;

    VAO = 0;

    unsigned int VBO = 0, EBO = 0;
    gl->glGenVertexArrays(1, &VAO);
    gl->glGenBuffers(1, &VBO);
    gl->glGenBuffers(1, &EBO);

    gl->glBindVertexArray(VAO);

    std::cout << m_model.m_positions.size() << " is the number of verts." << std::endl;;

    gl->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_model.m_positions.size(), m_model.m_positions.data(), GL_STATIC_DRAW);

    std::vector<float> indices;

    uint32_t i = 0;
    for(auto& batch : m_model.m_batches)
    {
        for(auto& pkt : batch.packets)
        {
            nTris += pkt.primitives.size();

            for(auto& primitive : pkt.primitives)
            {
                //indices.reserve(indices.size() + primitive.numIndices);

                for(uint32_t index : primitive.positionIndices)
                {
                    indices.push_back(index);
                    i++;
                }
            }
        }
    }

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    gl->glEnableVertexAttribArray(0);

    // texture coordinates
    //gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //gl->glEnableVertexAttribArray(1);

    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ObjectRenderer::draw()
{
    GalaxyRenderer::gl->glBindVertexArray(VAO);
    GalaxyRenderer::gl->glDrawElements(GL_TRIANGLES, nTris, GL_UNSIGNED_INT, 0);

}

