#pragma once

#include "io/BaseFile.h"

#include "rendering/GX.h"

#include <vector>
#include <array>
#include <glm/glm.hpp>

#include <QColor>

class BmdFile
{
    // INF1
    enum J3DLoadFlags {
        // Scaling rule
        ScalingRule_Basic = 0x00000000,
        ScalingRule_XSI   = 0x00000001,
        ScalingRule_Maya  = 0x00000002,
        ScalingRule_Mask  = 0x0000000F,

        // TODO(jstpierre): Document the other bits.
    };

    struct INF1
    {
        VectorView<uint8_t> hierarchyData;
        J3DLoadFlags loadFlags;
    };



    // MAT3
    enum TexMatrixProjection {
        MTX3x4 = 0,
        MTX2x4 = 1,
    };

    struct TexMatrix
    {
        bool isNull = true;

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

        std::vector<TexMatrix> texMatrices; // let's leak all the memory!
        std::vector<float> indTexMatrices;

        GX::Material gxMaterial;


        std::vector<QColor> colorMatRegs;
        std::vector<QColor> colorAmbRegs;
        std::vector<QColor> colorConstants;
        std::vector<QColor> colorRegisters;

        GX::FogBlock fogBlock;
    };

    struct Sampler
    {
        uint32_t index;

        QString name;

        GX::WrapMode_t wrapS;
        GX::WrapMode_t wrapT;

        GX::TexFilter_t minFilter;
        GX::TexFilter_t magFilter;

        float minLOD;
        float maxLOD;
        float lodBias;
        int32_t textureDataIndex;
    };

    // actual class starts here
    BaseFile* file;

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

    // INF1
    INF1 inf1;

    // MAT3
    std::vector<Material> m_materials;

    // TEX1
    std::vector<GX::BTI_Texture> m_textures;
    std::vector<Sampler> m_samplers;
};
