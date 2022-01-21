#pragma once

#include "io/BaseFile.h"

#include "rendering/GX.h"

#include <vector>
#include <array>
#include <glm/glm.hpp>

#include <QColor>
#include <QByteArrayView>

class BmdFile
{
    struct SceneGraphNode
    {
        uint16_t materialID = -1;

        int16_t parentIndex;
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

    enum TexMatrixProjection {
        MTX3x4 = 0,
        MTX2x4 = 1,
    };

    struct TexMatrix
    {
        uint32_t info;
        TexMatrixProjection projection;
        glm::mat4 effectMatrix;
        glm::mat4 matrix;
    };

    struct Material
    {
        uint32_t index;
        QString name;
        uint32_t materialMode;
        bool translucent;

        std::vector<int16_t> indices;

        std::vector<TexMatrix*> texMatrices; // TODO vector of optionals maybe?
        std::vector<float> indTexMatrices;

        GX::Material gxMaterial;


        std::vector<QColor> colorMatRegs;
        std::vector<QColor> colorAmbRegs;
        std::vector<QColor> colorConstants;
        std::vector<QColor> colorRegisters;

        GX::FogBlock fogBlock;

        ~Material();
    };

    // The way this works is a bit complicated. Basically, textures can have different
    // LOD or wrap modes but share the same literal texture data. As such, we do a bit
    // of remapping here. TEX1_TextureData contains the texture data parameters, and
    // TEX1_Sampler contains the "sampling" parameters like LOD or wrap mode, along with
    // its associated texture data. Each texture in the TEX1 chunk is turned into a
    // TEX1_Surface.
    struct TextureData { // TODO naming style
        // The name can be used for external lookups and is required.
        QString name;
        uint32_t width;
        uint32_t height;
        GX::TexFormat format;
        uint32_t mipCount;
        QByteArrayView data;
        GX::TexPalette paletteFormat;
        QByteArrayView paletteData;
    };

    struct Sampler {
        uint32_t index;

        QString name;

        GX::WrapMode wrapS;
        GX::WrapMode wrapT;

        GX::TexFilter minFilter;
        GX::TexFilter magFilter;

        float minLOD;
        float maxLOD;
        float lodBias;
        int32_t textureDataIndex;
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

    // MAT3
    std::vector<Material> m_materials;

    // TEX1
    std::vector<TextureData> m_textureDatas;
    std::vector<Sampler> m_samplers;

    void readINF1();
    void readVTX1();
    void readEVP1();
    void readDRW1();
    void readJNT1();
    void readSHP1();
    void readMAT3();
    void readMDL3();
    void readTEX1();

    GX::BTI_Texture readBTI(uint32_t absoluteStartIndex, const QString& name);

    std::vector<QString> readStringTable(uint32_t absoluteOffset);

    float readArrayShort(uint8_t fixedPoint);
    float readArrayFloat();
    float readArrayValue(uint32_t type, uint8_t fixedPoint);

    QColor readColorValue(uint32_t type);
    QColor readColor_RGBA8();
    QColor readColor_RGBX8();
    QColor readColor_RGBA16();

    GX::ColorChannelControl readColorChannel(uint32_t absoluteColorChanTableOffset, uint16_t colorChanIndex);

    glm::vec3 readVec3();

public:
    BmdFile() = default;

    BmdFile(BaseFile* inRarcFile);

    void save();
    void close();
};
