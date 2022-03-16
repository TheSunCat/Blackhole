#pragma once

#include <QString>
#include <QColor>

#include <glm/glm.hpp>
#include <vector>
#include <span>

namespace GX
{

// huge thanks to noclip.website for these data structures!

enum class TexFormat {
    I4 = 0x0,
    I8 = 0x1,
    IA4 = 0x2,
    IA8 = 0x3,
    RGB565 = 0x4,
    RGB5A3 = 0x5,
    RGBA8 = 0x6,
    C4 = 0x8,
    C8 = 0x9,
    C14X2 = 0xA,
    CMPR = 0xE, /*!< Compressed */
};

enum class TexPalette {
    IA8 = 0x00,
    RGB565 = 0x01,
    RGB5A3 = 0x02,
};

enum class WrapMode {
    CLAMP = 0,
    REPEAT = 1,
    MIRROR = 2,
};

// ugh Windows.h
#ifdef NEAR
#undef NEAR
#endif

enum class TexFilter {
    NEAR = 0,               // Point sampling, no mipmap
    LINEAR = 1,             // Bilinear filtering, no mipmap
    NEAR_MIP_NEAR = 2,      // Point sampling, discrete mipmap
    LIN_MIP_NEAR = 3,       // Bilinear filtering, discrete mipmap
    NEAR_MIP_LIN = 4,       // Point sampling, linear mipmap
    LIN_MIP_LIN = 5,        // Trilinear filtering
};

enum class CullMode {
    NONE = 0,               // Do not cull any primitives.
    FRONT = 1,              // Cull front-facing primitives.
    BACK = 2,               // Cull back-facing primitives.
    ALL = 3,                // Cull all primitives.
};

enum class ColorSrc {
    REG = 0,
    VTX = 1,
};

enum class DiffuseFunction {
    NONE = 0x00,
    SIGN = 0x01,
    CLAMP = 0x02,
};

enum class AttenuationFunction {
    SPEC = 0x00, // Specular attenuation
    SPOT = 0x01, // Distance/spotlight attenuation
    NONE,
};

struct ColorChannelControl {
    bool lightingEnabled;
    ColorSrc matColorSource;
    ColorSrc ambColorSource;
    uint32_t litMask;
    DiffuseFunction diffuseFunction;
    AttenuationFunction attenuationFunction;
};

struct LightChannelControl
{
    ColorChannelControl alphaChannel;
    ColorChannelControl colorChannel;
};

enum class TexMtxMapMode {
    None = 0x00,
    // Uses "Basic" conventions, no -1...1 remap.
    // Peach Beach uses EnvmapBasic, not sure on what yet...
    EnvmapBasic = 0x01,
    ProjmapBasic = 0x02,
    ViewProjmapBasic = 0x03,
    // Unknown: 0x04, 0x05. No known uses.
    // Uses "Old" conventions, remaps translation in fourth component
    // TODO(jstpierre): Figure out the geometric interpretation of old vs. new
    EnvmapOld = 0x06,
    // Uses "New" conventions, remaps translation in third component
    Envmap = 0x07,
    Projmap = 0x08,
    ViewProjmap = 0x09,
    // Environment map, but based on a custom effect matrix instead of the default view
    // matrix. Used by certain actors in Wind Waker, like zouK1 in Master Sword Chamber.
    EnvmapOldEffectMtx = 0x0A,
    EnvmapEffectMtx = 0x0B,
};

enum class TexGenType {
    MTX3x4 = 0,
    MTX2x4 = 1,
    BUMP0 = 2,
    BUMP1 = 3,
    BUMP2 = 4,
    BUMP3 = 5,
    BUMP4 = 6,
    BUMP5 = 7,
    BUMP6 = 8,
    BUMP7 = 9,
    SRTG = 10,
};

enum class TexGenSrc {
    POS = 0,
    NRM = 1,
    BINRM = 2,
    TANGENT = 3,
    TEX0 = 4,
    TEX1 = 5,
    TEX2 = 6,
    TEX3 = 7,
    TEX4 = 8,
    TEX5 = 9,
    TEX6 = 10,
    TEX7 = 11,
    TEXCOORD0 = 12,
    TEXCOORD1 = 13,
    TEXCOORD2 = 14,
    TEXCOORD3 = 15,
    TEXCOORD4 = 16,
    TEXCOORD5 = 17,
    TEXCOORD6 = 18,
    COLOR0 = 19,
    COLOR1 = 20,
};

enum class PosNrmMatrix {
    PNMTX0 = 0,
    PNMTX1 = 3,
    PNMTX2 = 6,
    PNMTX3 = 9,
    PNMTX4 = 12,
    PNMTX5 = 15,
    PNMTX6 = 18,
    PNMTX7 = 21,
    PNMTX8 = 24,
    PNMTX9 = 27,
};

enum class TexGenMatrix {
    IDENTITY = 60,
    TEXMTX0 = 30,
    TEXMTX1 = 33,
    TEXMTX2 = 36,
    TEXMTX3 = 39,
    TEXMTX4 = 42,
    TEXMTX5 = 45,
    TEXMTX6 = 48,
    TEXMTX7 = 51,
    TEXMTX8 = 54,
    TEXMTX9 = 57,

    // Clever games can use PNMTX as inputs to texgen.
    PNMTX0 = 0,
    PNMTX1 = 3,
    PNMTX2 = 6,
    PNMTX3 = 9,
    PNMTX4 = 12,
    PNMTX5 = 15,
    PNMTX6 = 18,
    PNMTX7 = 21,
    PNMTX8 = 24,
    PNMTX9 = 27,
};

enum class PostTexGenMatrix {
    PTTEXMTX0  = 64,
    PTTEXMTX1  = 67,
    PTTEXMTX2  = 70,
    PTTEXMTX3  = 73,
    PTTEXMTX4  = 76,
    PTTEXMTX5  = 79,
    PTTEXMTX6  = 82,
    PTTEXMTX7  = 85,
    PTTEXMTX8  = 88,
    PTTEXMTX9  = 91,
    PTTEXMTX10 = 94,
    PTTEXMTX11 = 97,
    PTTEXMTX12 = 100,
    PTTEXMTX13 = 103,
    PTTEXMTX14 = 106,
    PTTEXMTX15 = 109,
    PTTEXMTX16 = 112,
    PTTEXMTX17 = 115,
    PTTEXMTX18 = 118,
    PTTEXMTX19 = 121,
    PTIDENTITY = 125,
};

struct TexGen {
    TexGenType type;
    TexGenSrc source;
    TexGenMatrix matrix;
    bool normalize;
    PostTexGenMatrix postMatrix;
};

enum class TevOp {
    ADD = 0,
    SUB = 1,
    COMP_R8_GT = 8,
    COMP_R8_EQ = 9,
    COMP_GR16_GT = 10,
    COMP_GR16_EQ = 11,
    COMP_BGR24_GT = 12,
    COMP_BGR24_EQ = 13,
    COMP_RGB8_GT = 14,
    COMP_RGB8_EQ = 15,
    COMP_A8_GT = COMP_RGB8_GT,
    COMP_A8_EQ = COMP_RGB8_EQ,
};

enum class TevBias {
    ZERO = 0,
    ADDHALF = 1,
    SUBHALF = 2,

    // Used to denote the compare ops to the HW.
    $HWB_COMPARE = 3,
};

enum class TevScale {
    SCALE_1 = 0,
    SCALE_2 = 1,
    SCALE_4 = 2,
    DIVIDE_2 = 3,

    // Used to denote the width of the compare op.
    $HWB_R8 = 0,
    $HWB_GR16 = 1,
    $HWB_BGR24 = 2,
    $HWB_RGB8 = 3,
};

enum class CC {
    CPREV = 0,              // Use the color value from previous TEV stage
    APREV = 1,              // Use the alpha value from previous TEV stage
    C0 = 2,                 // Use the color value from the color/output register 0
    A0 = 3,                 // Use the alpha value from the color/output register 0
    C1 = 4,                 // Use the color value from the color/output register 1
    A1 = 5,                 // Use the alpha value from the color/output register 1
    C2 = 6,                 // Use the color value from the color/output register 2
    A2 = 7,                 // Use the alpha value from the color/output register 2
    TEXC = 8,               // Use the color value from texture
    TEXA = 9,               // Use the alpha value from texture
    RASC = 10,              // Use the color value from rasterizer
    RASA = 11,              // Use the alpha value from rasterizer
    ONE = 12,
    HALF = 13,
    KONST = 14,
    ZERO = 15,              // Use to pass zero value
};

enum class CA {
    APREV = 0,              // Use the alpha value from previous TEV stage
    A0 = 1,                 // Use the alpha value from the color/output register 0
    A1 = 2,                 // Use the alpha value from the color/output register 1
    A2 = 3,                 // Use the alpha value from the color/output register 2
    TEXA = 4,               // Use the alpha value from texture
    RASA = 5,               // Use the alpha value from rasterizer
    KONST = 6,
    ZERO = 7,               // Use to pass zero value
};

enum class Register {
    PREV = 0,
    REG0 = 1,
    REG1 = 2,
    REG2 = 3,
};

enum class TexCoordID : uint32_t {
    TEXCOORD0 = 0,
    TEXCOORD1 = 1,
    TEXCOORD2 = 2,
    TEXCOORD3 = 3,
    TEXCOORD4 = 4,
    TEXCOORD5 = 5,
    TEXCOORD6 = 6,
    TEXCOORD7 = 7,
    TEXCOORD_NULL = 0xFF,
};

enum class TexMapID {
    TEXMAP0 = 0,
    TEXMAP1 = 1,
    TEXMAP2 = 2,
    TEXMAP3 = 3,
    TEXMAP4 = 4,
    TEXMAP5 = 5,
    TEXMAP6 = 6,
    TEXMAP7 = 7,
    TEXMAP_NULL = 0xFF,
};

enum class ColorChannelID {
    COLOR0 = 0,
    COLOR1 = 1,
    ALPHA0 = 2,
    ALPHA1 = 3,
    COLOR0A0 = 4,
    COLOR1A1 = 5,
    COLOR_ZERO = 6,
    ALPHA_BUMP = 7,
    ALPHA_BUMP_N = 8,
    COLOR_NULL = 0xFF,
};

enum class RasColorChannelID {
    COLOR0A0     = 0,
    COLOR1A1     = 1,
    ALPHA_BUMP   = 5,
    ALPHA_BUMP_N = 6,
    COLOR_ZERO   = 7,
};

enum class KonstColorSel {
    KCSEL_1   = 0x00,       // constant 1.0
    KCSEL_7_8 = 0x01,       // constant 7/8
    KCSEL_6_8 = 0x02,       // constant 6/8
    KCSEL_5_8 = 0x03,       // constant 5/8
    KCSEL_4_8 = 0x04,       // constant 4/8
    KCSEL_3_8 = 0x05,       // constant 3/8
    KCSEL_2_8 = 0x06,       // constant 2/8
    KCSEL_1_8 = 0x07,       // constant 1/8
    KCSEL_K0  = 0x0C,       // K0[RGB] register
    KCSEL_K1  = 0x0D,       // K1[RGB] register
    KCSEL_K2  = 0x0E,       // K2[RGB] register
    KCSEL_K3  = 0x0F,       // K3[RGB] register
    KCSEL_K0_R = 0x10,      //  K0[RRR] register
    KCSEL_K1_R = 0x11,      //  K1[RRR] register
    KCSEL_K2_R = 0x12,      //  K2[RRR] register
    KCSEL_K3_R = 0x13,      //  K3[RRR] register
    KCSEL_K0_G = 0x14,      //  K0[GGG] register
    KCSEL_K1_G = 0x15,      //  K1[GGG] register
    KCSEL_K2_G = 0x16,      //  K2[GGG] register
    KCSEL_K3_G = 0x17,      //  K3[GGG] register
    KCSEL_K0_B = 0x18,      //  K0[BBB] register
    KCSEL_K1_B = 0x19,      //  K1[BBB] register
    KCSEL_K2_B = 0x1A,      //  K2[BBB] register
    KCSEL_K3_B = 0x1B,      //  K3[RBB] register
    KCSEL_K0_A = 0x1C,      //  K0[AAA] register
    KCSEL_K1_A = 0x1D,      //  K1[AAA] register
    KCSEL_K2_A = 0x1E,      //  K2[AAA] register
    KCSEL_K3_A = 0x1F,      //  K3[AAA] register
};

enum class KonstAlphaSel {
    KASEL_1    = 0x00,      //  constant 1.0
    KASEL_7_8  = 0x01,      //  constant 7/8
    KASEL_6_8  = 0x02,      //  constant 6/8
    KASEL_5_8  = 0x03,      //  constant 5/8
    KASEL_4_8  = 0x04,      //  constant 4/8
    KASEL_3_8  = 0x05,      //  constant 3/8
    KASEL_2_8  = 0x06,      //  constant 2/8
    KASEL_1_8  = 0x07,      //  constant 1/8
    KASEL_K0_R = 0x10,      //  K0[R] register
    KASEL_K1_R = 0x11,      //  K1[R] register
    KASEL_K2_R = 0x12,      //  K2[R] register
    KASEL_K3_R = 0x13,      //  K3[R] register
    KASEL_K0_G = 0x14,      //  K0[G] register
    KASEL_K1_G = 0x15,      //  K1[G] register
    KASEL_K2_G = 0x16,      //  K2[G] register
    KASEL_K3_G = 0x17,      //  K3[G] register
    KASEL_K0_B = 0x18,      //  K0[B] register
    KASEL_K1_B = 0x19,      //  K1[B] register
    KASEL_K2_B = 0x1A,      //  K2[B] register
    KASEL_K3_B = 0x1B,      //  K3[B] register
    KASEL_K0_A = 0x1C,      //  K0[A] register
    KASEL_K1_A = 0x1D,      //  K1[A] register
    KASEL_K2_A = 0x1E,      //  K2[A] register
    KASEL_K3_A = 0x1F,      //  K3[A] register
};

enum class IndTexBiasSel {
    NONE = 0,
    S = 1,
    T = 2,
    ST = 3,
    U = 4,
    SU = 5,
    TU = 6,
    STU = 7,
};

enum class IndTexAlphaSel {
    OFF = 0,
    S = 1,
    T = 2,
    U = 3,
};

enum class IndTexFormat {
    _8 = 0,                 // 8-bit texture offset
    _5 = 1,                 // 5-bit texture offset
    _4 = 2,                 // 4-bit texture offset
    _3 = 3,                 // 3-bit texture offset
};

enum class IndTexWrap {
    OFF = 0,
    _256 = 1,
    _128 = 2,
    _64 = 3,
    _32 = 4,
    _16 = 5,
    _0 = 6,
};

enum class IndTexStageID {
    STAGE0 = 0,
    STAGE1 = 1,
    STAGE2 = 2,
    STAGE3 = 3,
};

enum class IndTexMtxID {
    OFF = 0,
    _0 = 1,
    _1 = 2,
    _2 = 3,
    S0 = 5,
    S1 = 6,
    S2 = 7,
    T0 = 9,
    T1 = 10,
    T2 = 11,
};

enum class TevColorChan {
    R = 0,
    G = 1,
    B = 2,
    A = 3,
};

typedef const std::array<TevColorChan, 4> SwapTable;

constexpr std::array<SwapTable, 4> tevDefaultSwapTables{
    SwapTable{TevColorChan::R, TevColorChan::G, TevColorChan::B, TevColorChan::A},
    SwapTable{TevColorChan::R, TevColorChan::R, TevColorChan::R, TevColorChan::A},
    SwapTable{TevColorChan::G, TevColorChan::G, TevColorChan::G, TevColorChan::A},
    SwapTable{TevColorChan::B, TevColorChan::B, TevColorChan::B, TevColorChan::A}
};

struct TevStage {
    CC colorInA, colorInB, colorInC, colorInD;
    TevOp colorOp;
    TevBias colorBias;
    TevScale colorScale;
    bool colorClamp;
    Register colorRegID;

    CA alphaInA, alphaInB, alphaInC, alphaInD;
    TevOp alphaOp;
    TevBias alphaBias;
    TevScale alphaScale;
    bool alphaClamp;
    Register alphaRegID;

    // SetTevOrder
    TexCoordID texCoordID;
    TexMapID texMap;
    RasColorChannelID channelID;

    KonstColorSel konstColorSel;
    KonstAlphaSel konstAlphaSel;

    // SetTevSwapMode / SetTevSwapModeTable
    SwapTable rasSwapTable; // TODO optional
    SwapTable texSwapTable; // TODO optional

    // SetTevIndirect
    IndTexStageID indTexStage;
    IndTexFormat indTexFormat;
    IndTexBiasSel indTexBiasSel;
    IndTexAlphaSel indTexAlphaSel;
    IndTexMtxID indTexMatrix;
    IndTexWrap indTexWrapS;
    IndTexWrap indTexWrapT;
    bool indTexAddPrev;
    bool indTexUseOrigLOD;
};

enum class IndTexScale {
    _1 = 0,
    _2 = 1,
    _4 = 2,
    _8 = 3,
    _16 = 4,
    _32 = 5,
    _64 = 6,
    _128 = 7,
    _256 = 8,
};

struct IndTexStage {
    TexCoordID texCoordId;
    TexMapID texture;
    IndTexScale scaleS;
    IndTexScale scaleT;
};

enum class AlphaOp {
    AND = 0,
    OR = 1,
    XOR = 2,
    XNOR = 3,
};

enum class CompareType {
    NEVER = 0,
    LESS = 1,
    EQUAL = 2,
    LEQUAL = 3,
    GREATER = 4,
    NEQUAL = 5,
    GEQUAL = 6,
    ALWAYS = 7,
};

struct AlphaTest {
    AlphaOp op;
    CompareType compareA;
    uint32_t referenceA;
    CompareType compareB;
    uint32_t referenceB;
};

enum class FogType {
    NONE          = 0x00,

    PERSP_LIN     = 0x02,
    PERSP_EXP     = 0x04,
    PERSP_EXP2    = 0x05,
    PERSP_REVEXP  = 0x06,
    PERSP_REVEXP2 = 0x07,

    ORTHO_LIN     = 0x0A,
    ORTHO_EXP     = 0x0C,
    ORTHO_EXP2    = 0x0D,
    ORTHO_REVEXP  = 0x0E,
    ORTHO_REVEXP2 = 0x0F,
};

enum class BlendMode {
    NONE = 0,
    BLEND = 1,
    LOGIC = 2,
    SUBTRACT = 3,
};

enum class BlendFactor {
    ZERO = 0,
    ONE = 1,
    SRCCLR = 2,
    INVSRCCLR = 3,
    SRCALPHA = 4,
    INVSRCALPHA = 5,
    DSTALPHA = 6,
    INVDSTALPHA = 7,
};

enum class LogicOp {
    CLEAR = 0,
    AND = 1,
    REVAND = 2,
    COPY = 3,
    INVAND = 4,
    NOOP = 5,
    XOR = 6,
    OR = 7,
    NOR = 8,
    EQUIV = 9,
    INV = 10,
    REVOR = 11,
    INVCOPY = 12,
    INVOR = 13,
    NAND = 14,
    SET = 15,
};

struct RopInfo {
    FogType fogType;
    bool fogAdjEnabled;

    bool depthTest;
    CompareType depthFunc;
    bool depthWrite;

    BlendMode blendMode;
    BlendFactor blendSrcFactor;
    BlendFactor blendDstFactor;
    LogicOp blendLogicOp;

    bool colorUpdate;
    bool alphaUpdate;

    uint32_t dstAlpha; // TODO optional
};

struct Material {
    // Debugging & ID
    QString name;

    // Polygon state
    CullMode cullMode;

    // Vertex state
    std::vector<LightChannelControl> lightChannels;
    std::vector<TexGen> texGens;

    // TEV state
    std::vector<TevStage> tevStages;
    // Indirect TEV state
    std::vector<IndTexStage> indTexStages;

    // Raster / blend state.
    AlphaTest alphaTest;
    RopInfo ropInfo;

    // Optimization and other state (TODO OPTIONAL)
    bool usePnMtxIdx = true;
    std::vector<bool> useTexMtxIdx; // defaults to false
    bool hasPostTexMtxBlock = true;
    bool hasLightsBlock = true;
    bool hasFogBlock = false;
    bool hasDynamicAlphaTest = false;
};

struct FogBlock {
    QColor color = QColor(0, 0, 0, 0);
    uint32_t A = 0;
    uint32_t B = 0;
    uint32_t C = 0;
    std::array<uint16_t, 10> adjTable;
    uint32_t adjCenter = 0;
};

struct Light
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 distAtten; // TODO attenuation?
    glm::vec3 cosAtten;
    QColor color;
};

struct BTI_Texture {
    QString name;
    TexFormat format;

    uint16_t width, height;

    WrapMode wrapS, wrapT;

    TexFilter minFilter, magFilter;

    float minLOD, maxLOD;
    float lodBias;

    uint8_t mipCount;

    std::span<const uint8_t> data;

    TexPalette paletteFormat;
    std::span<const uint8_t> paletteData;
};

void autoOptimizeMaterial(Material& mat);

bool autoOptimizeMaterialHasPostTexMtxBlock(Material& mat);
bool autoOptimizeMaterialHasLightsBlock(Material& mat);
bool autoOptimizeMaterialHasFogBlock(Material& mat);

}; // end namespace GX
