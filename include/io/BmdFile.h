#pragma once

#include "io/BaseFile.h"

#include "rendering/GX.h"
#include "rendering/Material.h"

#include <vector>
#include <array>
#include <span>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <QColor>

class BmdFile
{
    // INF1
    enum class J3DLoadFlags
    {
        // Scaling rule
        ScalingRule_Basic = 0x00000000,
        ScalingRule_XSI   = 0x00000001,
        ScalingRule_Maya  = 0x00000002,
        ScalingRule_Mask  = 0x0000000F,

        // TODO(jstpierre): Document the other bits.
    };

    struct INF1
    {
        std::span<uint8_t> hierarchyData;
        J3DLoadFlags loadFlags;
    };

    // VTX1
    struct VTX1
    {
        std::unordered_map<GX::Attr_t, GX::VertexArray> vertexArrays;
    };

    // EVP1
    struct WeightedBone
    {
        float weight;
        uint16_t jointIndex;
    };

    struct Envelope
    {
        std::vector<WeightedBone> weightedBones;
    };

    struct EVP1
    {
        std::vector<Envelope> envelopes;
        std::vector<glm::mat4> inverseBinds;
    };

    // DRW1
    enum class DRW1MatrixKind
    {
        Joint = 0x00,
        Envelope = 0x01
    };

    struct DRW1Matrix
    {
        DRW1MatrixKind kind;
        uint16_t index;
    };

    struct DRW1
    {
        std::vector<DRW1Matrix> matrixDefinitions;
    };

    // JNT1
    struct AABB // TODO maybe put this in Util or GX
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct JointTransformInfo
    {
        glm::vec3 scale = glm::vec3(1.0f);
        glm::vec3 translation;
        glm::quat rotation;

        void lerp(const JointTransformInfo& a, const JointTransformInfo& b, float t)
        {
            scale = Util::lerp(a.scale, b.scale, t);
            rotation = glm::slerp(a.rotation, b.rotation, t);
            translation = Util::lerp(a.translation, b.translation, t);
        }
    };

    struct Joint
    {
        QString name;
        JointTransformInfo transform;
        float boundingSphereRadius;
        AABB boundingBox;
        uint8_t calcFlags;
    };

    struct JNT1
    {
        std::vector<Joint> joints;
    };

    // SHP1
    // It is possible for the vertex display list to include indirect load commands, which request a synchronous
    // DMA into graphics memory from main memory. This is the standard way of doing vertex skinning in NW4R, for
    // instance, but it can be seen in other cases too. We handle this by splitting the data into multiple draw
    // commands per display list, which are the "LoadedVertexDraw" structures.

    // Note that the loader relies the common convention of the indexed load commands to produce the matrix tables
    // in each LoadedVertexDraw. GX establishes the conventions:
    //
    //  INDX_A = Position Matrices (=> posMatrixTable)
    //  INDX_B = Normal Matrices (currently unsupported)
    //  INDX_C = Texture Matrices (=> texMatrixTable)
    //  INDX_D = Light Objects (currently unsupported)

    struct LoadedVertexDraw // TODO are these types correct?
    {
        uint32_t indexOffset;
        uint32_t indexCount;
        std::vector<float> posMatrixTable;
        std::vector<float> texMatrixTable;
    };

    struct DrawCall;

    struct LoadedVertexData // TODO is ArrayBufferLike just a vector?
    {
        std::vector<uint32_t> indexData;
        std::vector<std::vector<float>> vertexBuffers; // TODO probs floats
        uint32_t totalIndexCount;
        uint32_t totalVertexCount;
        uint32_t vertexID;
        std::vector<LoadedVertexDraw> draws;

        // Internal. Used for re-running vertices
        // TODO DataView* dlView;
        std::vector<DrawCall>* drawCalls;
    };

    struct SingleVertexInputLayout
    {
        GXShaderLibrary::VertexAttributeInput attrInput; // TODO organize these structs & enums so they're not accross three files
        uint32_t bufferOffset; // TODO type?
        uint32_t bufferIndex;  // TODO type?
        GXShaderLibrary::GfxFormat format;
    };

    struct LoadedVertexLayout {
        GXShaderLibrary::GfxFormat indexFormat;
        std::vector<uint32_t> vertexBufferStrides; // TODO type?
        std::vector<SingleVertexInputLayout> singleVertexInputLayouts;

        // Precalculated offsets and formats for each attribute, for convenience filling buffers...
        std::vector<uint32_t> vertexAttributeOffsets;
        std::vector<GXShaderLibrary::GfxFormat> vertexAttributeFormats;
    };

    struct MtxGroup
    {
        std::vector<uint16_t> useMtxTable;
        uint32_t indexOffset; // TODO is this the type?
        uint32_t indexCount;  // TODO is this the type?
        LoadedVertexData loadedVertexData;
    };

    enum class ShapeDisplayFlags
    {
        NORMAL = 0,
        BILLBOARD = 1,
        Y_BILLBOARD = 2,
        MULTI = 3
    };

    struct Shape
    {
        ShapeDisplayFlags displayFlags;
        LoadedVertexLayout loadedVertexLayout;
        std::vector<MtxGroup> mtxGroups;
        AABB bbox;
        float boundingSphereRadius;
        uint32_t materialIndex; // TODO type?
    };

    struct SHP1
    {
        GX::VtxAttrFmt fmt;
        std::vector<Shape> shapes;
    };

    // MAT3
    enum class TexMatrixProjection
    {
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

    // TODO break these out maybe

    // INF1
    INF1 inf1;

    // VTX1
    VTX1 vtx1;

    // EVP1
    EVP1 evp1;

    // DRW1
    DRW1 drw1;

    // JNT1
    JNT1 jnt1;

    // MAT3
    std::vector<Material> m_materials;

    // TEX1
    std::vector<GX::BTI_Texture> m_textures;
    std::vector<Sampler> m_samplers;
};
