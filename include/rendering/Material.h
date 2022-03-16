#pragma once

#include "GX.h"

namespace GXShaderLibrary
{
    using namespace GX;

    constexpr int EFB_WIDTH = 640;
    constexpr int EFB_HEIGHT= 528;

    constexpr const char* tevOverflow = R"(
float TevOverflow(float a) { return float(int(a * 255.0) & 255) / 255.0; }
vec3 TevOverflow(vec3 a) { return vec3(TevOverflow(a.r), TevOverflow(a.g), TevOverflow(a.b)); }
vec4 TevOverflow(vec4 a) { return vec4(TevOverflow(a.r), TevOverflow(a.g), TevOverflow(a.b), TevOverflow(a.a)); }
)";

    enum class VertexAttributeInput {
        // TEXnMTXIDX are packed specially because of GL limitations.
        TEX0123MTXIDX,
        TEX4567MTXIDX,
        POS,
        NRM,
        // These are part of NBT in original GX. We pack them as separate inputs.
        BINRM,
        TANGENT,
        CLR0,
        CLR1,
        TEX01,
        TEX23,
        TEX45,
        TEX67,
        COUNT,
    };

    enum class FormatTypeFlags {
        U8 = 0x01,
        U16,
        U32,
        S8,
        S16,
        S32,
        F16,
        F32,

        // Compressed texture formats.
        BC1 = 0x41,
        BC2,
        BC3,
        BC4_UNORM,
        BC4_SNORM,
        BC5_UNORM,
        BC5_SNORM,

        // Special-case packed texture formats.
        U16_PACKED_5551 = 0x61,

        // Depth/stencil texture formats.
        D24 = 0x81,
        D32F,
        D24S8,
        D32FS8,
    };

    enum class FormatCompFlags {
        R    = 0x01,
        RG   = 0x02,
        RGB  = 0x03,
        RGBA = 0x04,
    };

    enum class FormatFlags {
        None         = 0b00000000,
        Normalized   = 0b00000001,
        sRGB         = 0b00000010,
        Depth        = 0b00000100,
        Stencil      = 0b00001000,
        RenderTarget = 0b00010000,
    };

    enum class GfxFormat;

    constexpr GfxFormat makeFormat(FormatTypeFlags type, FormatCompFlags comp, FormatFlags flags) {
        return GfxFormat((uint8_t(type) << 16) | (uint8_t(comp) << 8) | uint8_t(flags));
    }

    enum class GfxFormat {
        F16_R           = uint8_t(makeFormat(FormatTypeFlags::F16, FormatCompFlags::R,    FormatFlags::None)),
        F16_RG          = uint8_t(makeFormat(FormatTypeFlags::F16, FormatCompFlags::RG,   FormatFlags::None)),
        F16_RGB         = uint8_t(makeFormat(FormatTypeFlags::F16, FormatCompFlags::RGB,  FormatFlags::None)),
        F16_RGBA        = uint8_t(makeFormat(FormatTypeFlags::F16, FormatCompFlags::RGBA, FormatFlags::None)),
        F32_R           = uint8_t(makeFormat(FormatTypeFlags::F32, FormatCompFlags::R,    FormatFlags::None)),
        F32_RG          = uint8_t(makeFormat(FormatTypeFlags::F32, FormatCompFlags::RG,   FormatFlags::None)),
        F32_RGB         = uint8_t(makeFormat(FormatTypeFlags::F32, FormatCompFlags::RGB,  FormatFlags::None)),
        F32_RGBA        = uint8_t(makeFormat(FormatTypeFlags::F32, FormatCompFlags::RGBA, FormatFlags::None)),
        U8_R            = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::R,    FormatFlags::None)),
        U8_R_NORM       = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::R,    FormatFlags::Normalized)),
        U8_RG           = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RG,   FormatFlags::None)),
        U8_RG_NORM      = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RG,   FormatFlags::Normalized)),
        U8_RGB          = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RGB,  FormatFlags::None)),
        U8_RGB_NORM     = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RGB,  FormatFlags::Normalized)),
        U8_RGB_SRGB     = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RGB,  FormatFlags(uint8_t(FormatFlags::sRGB) | uint8_t(FormatFlags::Normalized)))),
        U8_RGBA         = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RGBA, FormatFlags::None)),
        U8_RGBA_NORM    = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RGBA, FormatFlags::Normalized)),
        U8_RGBA_SRGB    = uint8_t(makeFormat(FormatTypeFlags::U8,  FormatCompFlags::RGBA, FormatFlags(uint8_t(FormatFlags::sRGB) | uint8_t(FormatFlags::Normalized)))),
        U16_R           = uint8_t(makeFormat(FormatTypeFlags::U16, FormatCompFlags::R,    FormatFlags::None)),
        U16_R_NORM      = uint8_t(makeFormat(FormatTypeFlags::U16, FormatCompFlags::R,    FormatFlags::Normalized)),
        U16_RG_NORM     = uint8_t(makeFormat(FormatTypeFlags::U16, FormatCompFlags::RG,   FormatFlags::Normalized)),
        U16_RGBA_NORM   = uint8_t(makeFormat(FormatTypeFlags::U16, FormatCompFlags::RGBA, FormatFlags::Normalized)),
        U16_RGB         = uint8_t(makeFormat(FormatTypeFlags::U16, FormatCompFlags::RGB,  FormatFlags::None)),
        U32_R           = uint8_t(makeFormat(FormatTypeFlags::U32, FormatCompFlags::R,    FormatFlags::None)),
        U32_RG          = uint8_t(makeFormat(FormatTypeFlags::U32, FormatCompFlags::RG,   FormatFlags::None)),
        S8_R            = uint8_t(makeFormat(FormatTypeFlags::S8,  FormatCompFlags::R,    FormatFlags::None)),
        S8_R_NORM       = uint8_t(makeFormat(FormatTypeFlags::S8,  FormatCompFlags::R,    FormatFlags::Normalized)),
        S8_RG_NORM      = uint8_t(makeFormat(FormatTypeFlags::S8,  FormatCompFlags::RG,   FormatFlags::Normalized)),
        S8_RGB_NORM     = uint8_t(makeFormat(FormatTypeFlags::S8,  FormatCompFlags::RGB,  FormatFlags::Normalized)),
        S8_RGBA_NORM    = uint8_t(makeFormat(FormatTypeFlags::S8,  FormatCompFlags::RGBA, FormatFlags::Normalized)),
        S16_R           = uint8_t(makeFormat(FormatTypeFlags::S16, FormatCompFlags::R,    FormatFlags::None)),
        S16_RG          = uint8_t(makeFormat(FormatTypeFlags::S16, FormatCompFlags::RG,   FormatFlags::None)),
        S16_RG_NORM     = uint8_t(makeFormat(FormatTypeFlags::S16, FormatCompFlags::RG,   FormatFlags::Normalized)),
        S16_RGB_NORM    = uint8_t(makeFormat(FormatTypeFlags::S16, FormatCompFlags::RGB,  FormatFlags::Normalized)),
        S16_RGBA        = uint8_t(makeFormat(FormatTypeFlags::S16, FormatCompFlags::RGBA, FormatFlags::None)),
        S16_RGBA_NORM   = uint8_t(makeFormat(FormatTypeFlags::S16, FormatCompFlags::RGBA, FormatFlags::Normalized)),
        S32_R           = uint8_t(makeFormat(FormatTypeFlags::S32, FormatCompFlags::R,    FormatFlags::None)),

        // Packed texture formats.
        U16_RGBA_5551   = uint8_t(makeFormat(FormatTypeFlags::U16_PACKED_5551, FormatCompFlags::RGBA, FormatFlags::Normalized)),

        // Compressed
        BC1             = uint8_t(makeFormat(FormatTypeFlags::BC1,       FormatCompFlags::RGBA, FormatFlags::Normalized)),
        BC1_SRGB        = uint8_t(makeFormat(FormatTypeFlags::BC1,       FormatCompFlags::RGBA, FormatFlags(uint8_t(FormatFlags::Normalized) | uint8_t(FormatFlags::sRGB)))),
        BC2             = uint8_t(makeFormat(FormatTypeFlags::BC2,       FormatCompFlags::RGBA, FormatFlags::Normalized)),
        BC2_SRGB        = uint8_t(makeFormat(FormatTypeFlags::BC2,       FormatCompFlags::RGBA, FormatFlags(uint8_t(FormatFlags::Normalized) | uint8_t(FormatFlags::sRGB)))),
        BC3             = uint8_t(makeFormat(FormatTypeFlags::BC3,       FormatCompFlags::RGBA, FormatFlags::Normalized)),
        BC3_SRGB        = uint8_t(makeFormat(FormatTypeFlags::BC3,       FormatCompFlags::RGBA, FormatFlags(uint8_t(FormatFlags::Normalized) | uint8_t(FormatFlags::sRGB)))),
        BC4_UNORM       = uint8_t(makeFormat(FormatTypeFlags::BC4_UNORM, FormatCompFlags::R,    FormatFlags::Normalized)),
        BC4_SNORM       = uint8_t(makeFormat(FormatTypeFlags::BC4_SNORM, FormatCompFlags::R,    FormatFlags::Normalized)),
        BC5_UNORM       = uint8_t(makeFormat(FormatTypeFlags::BC5_UNORM, FormatCompFlags::RG,   FormatFlags::Normalized)),
        BC5_SNORM       = uint8_t(makeFormat(FormatTypeFlags::BC5_SNORM, FormatCompFlags::RG,   FormatFlags::Normalized)),

        // Depth/Stencil
        D24             = uint8_t(makeFormat(FormatTypeFlags::D24,       FormatCompFlags::R,  FormatFlags::Depth)),
        D24_S8          = uint8_t(makeFormat(FormatTypeFlags::D24S8,     FormatCompFlags::RG, FormatFlags(uint8_t(FormatFlags::Depth) | uint8_t(FormatFlags::Stencil)))),
        D32F            = uint8_t(makeFormat(FormatTypeFlags::D32F,      FormatCompFlags::R,  FormatFlags::Depth)),
        D32F_S8         = uint8_t(makeFormat(FormatTypeFlags::D32FS8,    FormatCompFlags::RG, FormatFlags(uint8_t(FormatFlags::Depth) | uint8_t(FormatFlags::Stencil)))),

        // Special RT formats for preferred backend support.
        U8_RGB_RT       = uint8_t(makeFormat(FormatTypeFlags::U8,        FormatCompFlags::RGB,  FormatFlags(uint8_t(FormatFlags::RenderTarget) | uint8_t(FormatFlags::Normalized)))),
        U8_RGBA_RT      = uint8_t(makeFormat(FormatTypeFlags::U8,        FormatCompFlags::RGBA, FormatFlags(uint8_t(FormatFlags::RenderTarget) | uint8_t(FormatFlags::Normalized)))),
        U8_RGBA_RT_SRGB = uint8_t(makeFormat(FormatTypeFlags::U8,        FormatCompFlags::RGBA, FormatFlags(uint8_t(FormatFlags::RenderTarget) | uint8_t(FormatFlags::Normalized) | uint8_t(FormatFlags::sRGB)))),
    };

    struct VertexAttributeGenDef {
        VertexAttributeInput attrInput;
        const char* name;
        GfxFormat format;
    };

    struct std::array<VertexAttributeGenDef, 12> vtxAttributeGenDefs{
        VertexAttributeGenDef{ VertexAttributeInput::POS,           "Position",      GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::TEX0123MTXIDX, "TexMtx0123Idx", GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::TEX4567MTXIDX, "TexMtx4567Idx", GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::NRM,           "Normal",        GfxFormat::F32_RGB },
        VertexAttributeGenDef{ VertexAttributeInput::BINRM,         "Binormal",      GfxFormat::F32_RGB },
        VertexAttributeGenDef{ VertexAttributeInput::TANGENT,       "Tangent",       GfxFormat::F32_RGB },
        VertexAttributeGenDef{ VertexAttributeInput::CLR0,          "Color0",        GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::CLR1,          "Color1",        GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::TEX01,         "Tex01",         GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::TEX23,         "Tex23",         GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::TEX45,         "Tex45",         GfxFormat::F32_RGBA },
        VertexAttributeGenDef{ VertexAttributeInput::TEX67,         "Tex67",         GfxFormat::F32_RGBA },
    };

    constexpr uint8_t getVertexInputLocation(VertexAttributeInput attrInput) {
        return uint8_t(std::distance(vtxAttributeGenDefs.begin(), std::find_if(vtxAttributeGenDefs.begin(), vtxAttributeGenDefs.end(), [&](VertexAttributeGenDef& def) { return def.attrInput == attrInput; })));
    }

    constexpr VertexAttributeGenDef getVertexInputGenDef(VertexAttributeInput attrInput) {
        return *std::find_if(vtxAttributeGenDefs.begin(), vtxAttributeGenDefs.end(), [&](VertexAttributeGenDef& def) { return def.attrInput == attrInput; });
    }

    struct GXMaterialHacks { // TODO all these are std::optional
        bool disableTextures;
        bool disableVertexColors;
        bool disableLighting;
    };

    constexpr bool colorChannelsEqual(ColorChannelControl a, ColorChannelControl b) {
        if (a.lightingEnabled != b.lightingEnabled) return false;
        if (a.litMask != b.litMask) return false;
        if (a.ambColorSource != b.ambColorSource) return false;
        if (a.matColorSource != b.matColorSource) return false;
        if (a.attenuationFunction != b.attenuationFunction) return false;
        if (a.diffuseFunction != b.diffuseFunction) return false;
        return true;
    }

    QString generateBindingsDefinition(Material& material);

    uint32_t getMaterialParamsBlockSize(Material& material);
    uint32_t getDrawParamsBlockSize(Material& material);

    class GX_Program // TODO extends DeviceProgram gx_material.ts
    {
    public:
        uint32_t ub_SceneParams = 0;       // TODO static + what size?
        uint32_t ub_MaterialParams = 1;    // TODO static + what size?
        uint32_t ub_DrawParams = 2;        // TODO static + what size?

        QString name;

        GX_Program(Material& material);

    private:
        QString generateFloat(float f);
        QString generateMaterialSource(ColorChannelControl chan, uint32_t i);
        QString generateAmbientSource(ColorChannelControl chan, uint32_t i);
        QString generateLightDiffFn(ColorChannelControl chan, const QString& lightName);
        QString generateLightAttnFn(ColorChannelControl chan, const QString& lightName);
        QString generateColorChannel(ColorChannelControl chan, const QString& outputName, uint32_t i);
        QString generateLightChannel(LightChannelControl lightChannel, const QString& outputName, uint32_t i);

        QString generateLightChannels();

        QString generateMulPntMatrixStatic(TexGenMatrix_t pnt, const QString& src, const QString& funcName = "Mul"); // Output is a vec3, src is a vec4.
        QString generateMulPntMatrixDynamic(const QString& attrStr, const QString& src, const QString& funcName = "Mul"); // Output is a vec3, src is a vec4.
        QString generateTexMtxIdxAttr(TexCoordID_t index);

        // TexGen
        QString generateTexGenSource(TexGenSrc_t src);
        QString generatePostTexGenMatrixMult(TexGen texCoordGen, const QString& src);
        QString generateTexGenMatrixMult(uint32_t texCoordGenIndex, const QString& src);
        QString generateTexGenType(uint32_t texCoordGenIndex);
        QString generateTexGenNrm(uint32_t texCoordGenIndex);
        QString generateTexGenPost(uint32_t texCoordGenIndex);
        QString generateTexGen(uint32_t i);

        QString generateTexGens();
        QString generateTexCoordVaryings();
        QString generateTexCoordGetters();

        // IndTex
        QString generateIndTexStageScaleN(IndTexScale_t scale);
        QString generateIndTexStageScale(IndTexStage stage);
        QString generateTextureSample(uint32_t index, const QString& coord);
        QString generateIndTexStage(uint32_t indTexStageIndex);
        QString generateIndTexStages();

        // TEV
        QString generateKonstColorSel(KonstColorSel_t konstColor);
        QString generateKonstAlphaSel(KonstAlphaSel_t konstAlpha);
        QString generateIndTexCoordBase(TevStage& stage);
        QString generateAlphaBumpSelChannel(TevStage& stage);
        QString generateAlphaBumpSel(TevStage& stage);
        QString generateRas(TevStage& stage);
        bool stageUsesSimpleCoords(TevStage& stage);
        QString generateTexAccess(TevStage& stage);
        char generateComponentSwizzle(SwapTable* swapTable, TevColorChan_t colorIn);
        QString generateColorSwizzle(SwapTable* swapTable, CC_t colorIn);
        QString generateColorIn(TevStage& stage, CC_t colorIn);
        QString generateAlphaIn(TevStage& stage, CA_t alphaIn);
        QString generateTevInputs(TevStage& stage);
        QString generateTevRegister(Register_t regID);
        QString generateTevOpBiasScaleClamp(const QString& value, TevBias_t bias, TevScale_t scale);
        QString generateTevOp(TevOp_t op, TevBias_t bias, TevScale_t scale, const QString& a, const QString& b, const QString& c, const QString& d, const QString& zero);
        QString generateTevOpValue(TevOp_t op, TevBias_t bias, TevScale_t scale, bool clamp, const QString& a, const QString& b, const QString& c, const QString& d, const QString& zero);
        QString generateColorOp(TevStage& stage);
        QString generateAlphaOp(TevStage& stage);
        QString generateTevTexCoordWrapN(const QString& texCoord, IndTexWrap_t wrap);
        QString generateTevTexCoordWrap(TevStage& stage);
        QString generateTevTexCoordIndTexCoordBias(TevStage& stage);
        QString generateTevTexCoordIndTexCoord(TevStage& stage);
        QString generateTevTexCoordIndirectMtx(TevStage& stage);
        QString generateTevTexCoordIndirectTranslation(TevStage& stage);
        QString generateTevTexCoordIndirect(TevStage& stage);
        QString generateTevTexCoord(TevStage& stage);
        QString generateTevStage(uint32_t tevStageIndex);
        QString generateTevStages();
        QString generateTevStagesLastMinuteFixup();


        GXMaterialHacks hacks;

        Material material;
    };

}; // namespace GXShaderLibrary
