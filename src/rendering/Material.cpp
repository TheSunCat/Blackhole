#include "rendering/Material.h"

QString GXShaderLibrary::generateBindingsDefinition(Material material)
{
    return QString("\
// Expected to be constant across the entire scene.\
layout(std140) uniform ub_SceneParams {\
    Mat4x4 u_Projection;\
    vec4 u_Misc0;\
};\
#define u_SceneTextureLODBias u_Misc0[0]\
struct Light {\
    vec4 Color;\
    vec4 Position;\
    vec4 Direction;\
    vec4 DistAtten;\
    vec4 CosAtten;\
};\
struct FogBlock {\
    // A, B, C, Center\
    vec4 Param;\
    // 10 items\
    vec4 AdjTable[3];\
    // Fog color is RGB\
    vec4 Color;\
};\
// Expected to change with each material.\
layout(std140) uniform ub_MaterialParams {\
    vec4 u_ColorMatReg[2];\
    vec4 u_ColorAmbReg[2];\
    vec4 u_KonstColor[4];\
    vec4 u_Color[4];\
    Mat4x3 u_TexMtx[10];\
    vec4 u_TextureSizes[4];\
    vec4 u_TextureBiases[2];\
    Mat4x2 u_IndTexMtx[3];\
    // Optional parameters.") +

    QString(material.hasPostTexMtxBlock ? "\
    Mat4x3 u_PostTexMtx[20];" : "") +

    QString(material.hasLightsBlock ? "\
    Light u_LightParams[8];" : "") +

    QString(material.hasFogBlock ? "\
    FogBlock u_FogBlock;" : "") +

    QString(material.hasDynamicAlphaTest ? "\
    vec4 u_DynamicAlphaParams;" : "") +

    QString("\
};\
// Expected to change with each shape draw.\
layout(std140) uniform ub_DrawParams {\n") +

    QString(material.usePnMtxIdx ?
    "\nMat4x3 u_PosMtx[10];" : "\nMat4x3 u_PosMtx[1];") +

    QString("\
};\
uniform sampler2D u_Texture0;\
uniform sampler2D u_Texture1;\
uniform sampler2D u_Texture2;\
uniform sampler2D u_Texture3;\
uniform sampler2D u_Texture4;\
uniform sampler2D u_Texture5;\
uniform sampler2D u_Texture6;\
uniform sampler2D u_Texture7;\
");
}
