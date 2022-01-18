#pragma once

#include "io/BaseFile.h"

#include <vector>
#include <array>
#include <QColor>
#include <glm/glm.hpp>

class BmdFile
{
    struct SceneGraphNode
    {
        uint16_t materialID;

        uint32_t parentIndex;
        uint32_t nodeType; // 0: shape, 1: joint
        uint16_t nodeID;
    };

    struct MultiMatrix
    {
        uint32_t count;
        std::vector<uint16_t> indices;
        std::vector<glm::mat4> matrices;
        std::vector<float> weights;
    };

    struct MatrixType
    {
        bool weighted;
        uint16_t index;
    };

    struct Joint
    {
        uint16_t unk1;
        uint8_t unk2;

        glm::vec3 scale, rotation, translation;
        glm::mat4 matrix;
        glm::mat4 finalMatrix; // matrix with parents' transforms applied
    };

    struct Batch
    {
        struct Packet
        {
            struct Primitive
            {
                uint32_t numIndices;
                uint32_t primitiveType;

                uint32_t arrayMask;
                std::vector<uint32_t> posMatrixIndices;
                std::vector<uint32_t> positionIndices;
                std::vector<uint32_t> normalIndices;
                std::vector<std::vector<uint32_t>> colorIndices;
                std::vector<std::vector<uint32_t>> texcoordIndices;
            };


            std::vector<Primitive> primitives;
            std::vector<uint16_t> matrixTable;
        };


        uint8_t matrixType;

        std::vector<Packet> packets;

        float unk;
    };

    // actual class starts here
    BaseFile* file;


    // INF1
    uint32_t m_vertexCount;
    std::vector<SceneGraphNode> m_sceneGraph;

    // VTX1
    uint32_t m_arrayMask;
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::array<std::vector<QColor>, 2> m_colors;
    std::array<std::vector<glm::vec2>, 8> m_texCoords;

    // EVP1
    std::vector<MultiMatrix> m_multiMatrices;

    // DRW1
    std::vector<MatrixType> m_matrixTypes;

    // JNT1
    std::vector<Joint> m_joints;

    // SHP1
    std::vector<Batch> m_batches;

    void readINF1();
    void readVTX1();
    void readEVP1();
    void readDRW1();
    void readJNT1();
    void readSHP1();

    float readArrayShort(uint8_t fixedPoint);
    float readArrayFloat();
    float readArrayValue(uint32_t type, uint8_t fixedPoint);

    QColor readColorValue(uint32_t type);
    QColor readColor_RGBA8();
    QColor readColor_RGBX8();

    glm::vec3 readVec3();

public:
    BmdFile(BaseFile* inRarcFile);

    void save();
    void close();
};
