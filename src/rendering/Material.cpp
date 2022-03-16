#include "rendering/Material.h"

#include <iostream>

using namespace GX;

QString GXShaderLibrary::generateBindingsDefinition(Material& material)
{
    return QString(R"(
// Expected to be constant across the entire scene.
layout(std140) uniform ub_SceneParams {
    Mat4x4 u_Projection;
    vec4 u_Misc0;
};
#define u_SceneTextureLODBias u_Misc0[0]
struct Light {
    vec4 Color;
    vec4 Position;
    vec4 Direction;
    vec4 DistAtten;
    vec4 CosAtten;
};
struct FogBlock {
    // A, B, C, Center
    vec4 Param;
    // 10 items
    vec4 AdjTable[3];
    // Fog color is RGB
    vec4 Color;
};
// Expected to change with each material.
layout(std140) uniform ub_MaterialParams {
    vec4 u_ColorMatReg[2];
    vec4 u_ColorAmbReg[2];
    vec4 u_KonstColor[4];
    vec4 u_Color[4];
    Mat4x3 u_TexMtx[10];
    vec4 u_TextureSizes[4];
    vec4 u_TextureBiases[2];
    Mat4x2 u_IndTexMtx[3];
    // Optional parameters.)") +

    QString(material.hasPostTexMtxBlock ? "\
    Mat4x3 u_PostTexMtx[20];" : "") +

    QString(material.hasLightsBlock ? "\
    Light u_LightParams[8];" : "") +

    QString(material.hasFogBlock ? "\
    FogBlock u_FogBlock;" : "") +

    QString(material.hasDynamicAlphaTest ? "\
    vec4 u_DynamicAlphaParams;" : "") +

    QString(R"(
};
// Expected to change with each shape draw.
layout(std140) uniform ub_DrawParams {)") +

    QString(material.usePnMtxIdx ?
    "\nMat4x3 u_PosMtx[10];" : "\nMat4x3 u_PosMtx[1];") +

    QString(R"(
};
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform sampler2D u_Texture4;
uniform sampler2D u_Texture5;
uniform sampler2D u_Texture6;
uniform sampler2D u_Texture7;)");
}

uint32_t GXShaderLibrary::getMaterialParamsBlockSize(Material& material) {
    uint32_t size = 4*2 + 4*2 + 4*4 + 4*4 + 4*3*10 + 4*4 + 4*2 + 4*2*3;
    if (material.hasPostTexMtxBlock)
        size += 4*3*20;
    if (material.hasLightsBlock)
        size += 4*5*8;
    if (material.hasFogBlock)
        size += 4*5;
    if (material.hasDynamicAlphaTest)
        size += 4*1;

    return size;
}

uint32_t GXShaderLibrary::getDrawParamsBlockSize(Material& material) {
    uint32_t size = 0;

    if (material.usePnMtxIdx)
        size += 4*3 * 10;
    else
        size += 4*3 * 1;

    return size;
}

GXShaderLibrary::GX_Program::GX_Program(GX::Material& material) : name(material.name)
{
    // TODO generateShaders();
}

QString GXShaderLibrary::GX_Program::generateFloat(float f)
{
    std::string s = std::to_string(f);
    if (s.find_first_of('.') == std::string::npos)
        s += ".0";
    return QString::fromStdString(s);
}

QString GXShaderLibrary::GX_Program::generateMaterialSource(ColorChannelControl chan, uint32_t i) {
    //if (this.hacks !== null && this.hacks.disableVertexColors && chan.matColorSource === GX.ColorSrc.VTX)
    //    return "vec4(1.0, 1.0, 1.0, 1.0)";

    switch (chan.matColorSource) {
        case GX::ColorSrc::VTX: return QString("a_Color") + i;
        case GX::ColorSrc::REG: return QString("u_ColorMatReg[") + i + "]";
    }

    return "";
}

QString GXShaderLibrary::GX_Program::generateAmbientSource(ColorChannelControl chan, uint32_t i) {
    //if (this.hacks !== null && this.hacks.disableVertexColors && chan.ambColorSource === GX.ColorSrc.VTX)
    //    return "vec4(1.0, 1.0, 1.0, 1.0)";

    switch (chan.ambColorSource) {
        case GX::ColorSrc::VTX: return QString("a_Color") + i;
        case GX::ColorSrc::REG: return QString("u_ColorMatReg[") + i + "]";
    }

    return "";
}

QString GXShaderLibrary::GX_Program::generateLightDiffFn(ColorChannelControl chan, const QString& lightName)
{
    QString NdotL = "dot(t_Normal, t_LightDeltaDir)";

    switch (chan.diffuseFunction) {
        case GX::DiffuseFunction::NONE: return "1.0";
        case GX::DiffuseFunction::SIGN: return NdotL;
        case GX::DiffuseFunction::CLAMP: return "max(" + NdotL + ", 0.0)";
    }
}

QString GXShaderLibrary::GX_Program::generateLightAttnFn(ColorChannelControl chan, const QString& lightName) {
    if (chan.attenuationFunction == GX::AttenuationFunction::NONE) {
        return R"(
    t_Attenuation = 1.0;)";
    } else if (chan.attenuationFunction == GX::AttenuationFunction::SPOT) {
        QString attn = "max(0.0, dot(t_LightDeltaDir, " + lightName + ".Direction.xyz))";
        QString cosAttn = "max(0.0, ApplyAttenuation(" + lightName + ".CosAtten.xyz, " + attn + "))";
        QString distAttn = "dot(" + lightName + ".DistAtten.xyz, vec3(1.0, t_LightDeltaDist, t_LightDeltaDist2))";
        return "\n\tt_Attenuation = " + cosAttn + " / " + distAttn + ";)";
    } else if (chan.attenuationFunction == GX::AttenuationFunction::SPEC) {
        QString attn = "(dot(t_Normal, t_LightDeltaDir) >= 0.0) ? max(0.0, dot(t_Normal, " + lightName + ".Direction.xyz)) : 0.0";
        QString cosAttn = "ApplyAttenuation(" + lightName + ".CosAtten.xyz, t_Attenuation)";
        QString distAttn = "ApplyAttenuation(" + lightName + ".DistAtten.xyz, t_Attenuation)";
        return QString(R"(
    t_Attenuation = )") + attn + R"(;
    t_Attenuation = )" + cosAttn + " / " + distAttn + ";";
    } else {
        throw "whoops"; // lol nice
    }
}

QString GXShaderLibrary::GX_Program::generateColorChannel(ColorChannelControl chan, const QString& outputName, uint32_t i) {
        QString matSource = generateMaterialSource(chan, i);
        QString ambSource = generateAmbientSource(chan, i);

        bool lightingEnabled = chan.lightingEnabled;
        if (hacks.disableLighting)
            lightingEnabled = false;

        QString generateLightAccum = "";
        if (lightingEnabled) {
            generateLightAccum = "\n\tt_LightAccum = " + ambSource + ";)";

        if (chan.litMask != 0)
            assert(material.hasLightsBlock);

        for (uint8_t j = 0; j < 8; j++) {
            if (!(chan.litMask & (1 << j)))
                continue;

            QString lightName = QString("u_LightParams[") + j + "]";
            generateLightAccum += R"(
    t_LightDelta = )" + lightName + R"(.Position.xyz - v_Position.xyz;
    t_LightDeltaDist2 = dot(t_LightDelta, t_LightDelta);
    t_LightDeltaDist = sqrt(t_LightDeltaDist2);
    t_LightDeltaDir = t_LightDelta / t_LightDeltaDist;
)" + generateLightAttnFn(chan, lightName) + R"(
    t_LightAccum += )" + generateLightDiffFn(chan, lightName) + " * t_Attenuation * " + lightName + ".Color;\n";
        }
    } else {
        // Without lighting, everything is full-bright.
        generateLightAccum += "\n\tt_LightAccum = vec4(1.0);";
    }

    return generateLightAccum + "\n\t" + outputName + " = " + matSource + " * clamp(t_LightAccum, 0.0, 1.0);";
}

QString GXShaderLibrary::GX_Program::generateLightChannel(LightChannelControl lightChannel, const QString& outputName, uint32_t i) {
    if (colorChannelsEqual(lightChannel.colorChannel, lightChannel.alphaChannel)) {
        return "\n\t"
        + generateColorChannel(lightChannel.colorChannel, outputName, i);
    } else {
        return "\n\t" +
        generateColorChannel(lightChannel.colorChannel, "t_ColorChanTemp", i) + "\n\t" +
        outputName + ".rgb = t_ColorChanTemp.rgb;\n\t" +
        generateColorChannel(lightChannel.alphaChannel, "t_ColorChanTemp", i) + "\n\t" +
        outputName + ".a = t_ColorChanTemp.a;";
    }
}

QString GXShaderLibrary::GX_Program::generateLightChannels()
{
    QString ret = "";
    for(uint32_t i = 0; i < material.lightChannels.size(); i++)
        ret += generateLightChannel(material.lightChannels[i], QString("v_Color") + i, i) + "\n";

    return ret;
}

// Output is a vec3, src is a vec4.
QString GXShaderLibrary::GX_Program::generateMulPntMatrixStatic(TexGenMatrix_t pnt, const QString& src, const QString& funcName) {
    if (pnt == GX::TexGenMatrix::IDENTITY) {
        return src + ".xyz";
    } else if (pnt >= GX::TexGenMatrix::TEXMTX0) {
        QString texMtxIdx = QString::number((pnt - GX::TexGenMatrix::TEXMTX0) / 3);
        return funcName + "(u_TexMtx[" + texMtxIdx + "], " + src + ")";
    } else if (pnt >= GX::TexGenMatrix::PNMTX0) {
        QString pnMtxIdx = QString::number((pnt - GX::TexGenMatrix::PNMTX0) / 3);
        return funcName + "(u_PosMtx[" + pnMtxIdx + "], " + src + ")";
    } else {
        throw "whoops";
    }
}

// Output is a vec3, src is a vec4.
QString GXShaderLibrary::GX_Program::generateMulPntMatrixDynamic(const QString& attrStr, const QString& src, const QString& funcName) {
    return funcName + "(GetPosTexMatrix(" + attrStr + "), " + src + ")";
}

QString GXShaderLibrary::GX_Program::generateTexMtxIdxAttr(TexCoordID_t index) {
    if (index == TexCoordID::TEXCOORD0) return "(a_TexMtx0123Idx.x * 256.0)";
    if (index == TexCoordID::TEXCOORD1) return "(a_TexMtx0123Idx.y * 256.0)";
    if (index == TexCoordID::TEXCOORD2) return "(a_TexMtx0123Idx.z * 256.0)";
    if (index == TexCoordID::TEXCOORD3) return "(a_TexMtx0123Idx.w * 256.0)";
    if (index == TexCoordID::TEXCOORD4) return "(a_TexMtx4567Idx.x * 256.0)";
    if (index == TexCoordID::TEXCOORD5) return "(a_TexMtx4567Idx.y * 256.0)";
    if (index == TexCoordID::TEXCOORD6) return "(a_TexMtx4567Idx.z * 256.0)";
    if (index == TexCoordID::TEXCOORD7) return "(a_TexMtx4567Idx.w * 256.0)";
    throw "whoops";
}

// TexGen
QString GXShaderLibrary::GX_Program::generateTexGenSource(TexGenSrc_t src)
{
    switch (src) {
        case TexGenSrc::POS:       return "vec4(a_Position.xyz, 1.0)";
        case TexGenSrc::NRM:       return "vec4(a_Normal.xyz, 1.0)";
        case TexGenSrc::BINRM:     return "vec4(a_Binormal.xyz, 1.0)";
        case TexGenSrc::TANGENT:   return "vec4(a_Tangent.xyz, 1.0)";
        case TexGenSrc::COLOR0:    return "v_Color0";
        case TexGenSrc::COLOR1:    return "v_Color1";
        case TexGenSrc::TEX0:      return "vec4(a_Tex01.xy, 1.0, 1.0)";
        case TexGenSrc::TEX1:      return "vec4(a_Tex01.zw, 1.0, 1.0)";
        case TexGenSrc::TEX2:      return "vec4(a_Tex23.xy, 1.0, 1.0)";
        case TexGenSrc::TEX3:      return "vec4(a_Tex23.zw, 1.0, 1.0)";
        case TexGenSrc::TEX4:      return "vec4(a_Tex45.xy, 1.0, 1.0)";
        case TexGenSrc::TEX5:      return "vec4(a_Tex45.zw, 1.0, 1.0)";
        case TexGenSrc::TEX6:      return "vec4(a_Tex67.xy, 1.0, 1.0)";
        case TexGenSrc::TEX7:      return "vec4(a_Tex67.zw, 1.0, 1.0)";
        // Use a previously generated texcoordgen.
        case TexGenSrc::TEXCOORD0: return "vec4(v_TexCoord0, 1.0)";
        case TexGenSrc::TEXCOORD1: return "vec4(v_TexCoord1, 1.0)";
        case TexGenSrc::TEXCOORD2: return "vec4(v_TexCoord2, 1.0)";
        case TexGenSrc::TEXCOORD3: return "vec4(v_TexCoord3, 1.0)";
        case TexGenSrc::TEXCOORD4: return "vec4(v_TexCoord4, 1.0)";
        case TexGenSrc::TEXCOORD5: return "vec4(v_TexCoord5, 1.0)";
        case TexGenSrc::TEXCOORD6: return "vec4(v_TexCoord6, 1.0)";
        default:
            throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generatePostTexGenMatrixMult(TexGen texCoordGen, const QString& src)
{
    if (texCoordGen.postMatrix == PostTexGenMatrix::PTIDENTITY) {
        return src + ".xyz";
    } else if (texCoordGen.postMatrix >= PostTexGenMatrix::PTTEXMTX0) {
        QString texMtxIdx = QString::number((texCoordGen.postMatrix - PostTexGenMatrix::PTTEXMTX0) / 3);
        return "Mul(u_PostTexMtx[" + texMtxIdx + "], " + src + ")";
    } else {
        throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateTexGenMatrixMult(uint32_t texCoordGenIndex, const QString& src)
{
    if (material.useTexMtxIdx[texCoordGenIndex]) {
        QString attrStr = generateTexMtxIdxAttr(TexCoordID_t(texCoordGenIndex));
        return generateMulPntMatrixDynamic(attrStr, src);
    } else {
        return generateMulPntMatrixStatic(material.texGens[texCoordGenIndex].matrix, src);
    }
}

QString GXShaderLibrary::GX_Program::generateTexGenType(uint32_t texCoordGenIndex)
{
    TexGen texCoordGen = material.texGens[texCoordGenIndex];
    QString src = generateTexGenSource(texCoordGen.source);

    if (texCoordGen.type == TexGenType::SRTG)
        return "vec3(" + src + ".xy, 1.0)";
    else if (texCoordGen.type == TexGenType::MTX2x4)
        return "vec3(" + generateTexGenMatrixMult(texCoordGenIndex, src) + ".xy, 1.0)";
    else if (texCoordGen.type == TexGenType::MTX3x4)
        return generateTexGenMatrixMult(texCoordGenIndex, src);
    else
        throw "whoops";
}

QString GXShaderLibrary::GX_Program::generateTexGenNrm(uint32_t texCoordGenIndex)
{
    TexGen texCoordGen = material.texGens[texCoordGenIndex];
    QString src = generateTexGenType(texCoordGenIndex);

    if (texCoordGen.normalize)
        return "normalize(" + src + ")";
    else
        return src;
}

QString GXShaderLibrary::GX_Program::generateTexGenPost(uint32_t texCoordGenIndex)
{
    TexGen texCoordGen = material.texGens[texCoordGenIndex];
    QString src = generateTexGenNrm(texCoordGenIndex);

    if (texCoordGen.postMatrix == PostTexGenMatrix::PTIDENTITY) {
        return src;
    } else {
        return generatePostTexGenMatrixMult(texCoordGen, "vec4(" + src + ", 1.0)");
    }
}

QString GXShaderLibrary::GX_Program::generateTexGen(uint32_t i)
{
    TexGen tg = material.texGens[i];

    QString suffix;
    if (tg.type == TexGenType::MTX2x4 || tg.type == TexGenType::SRTG)
        suffix = ".xy";
    else if (tg.type == TexGenType::MTX3x4)
        suffix = ".xyz";
    else
        throw "whoops";

    return QString("\n\
// TexGen ") + i + " Type: " + tg.type + " Source: " + tg.source + " Matrix: " + tg.matrix + "\n\
v_TexCoord" + i + " = " + generateTexGenPost(i) + suffix + ";)";
}

QString GXShaderLibrary::GX_Program::generateTexGens()
{
    QString ret = "";
    for(uint32_t i = 0; i < material.texGens.size(); i++)
        ret += generateTexGen(i);

    return ret;
}

QString GXShaderLibrary::GX_Program::generateTexCoordVaryings()
{
    QString ret = "";
    for(uint32_t i = 0; i < material.texGens.size(); i++)
    {
        TexGen tg = material.texGens[i];

        if (tg.type == TexGenType::MTX2x4 || tg.type == TexGenType::SRTG)
            ret += QString("varying vec2 v_TexCoord") + i + ";\n";
        else if (tg.type == TexGenType::MTX3x4)
            ret += QString("varying highp vec3 v_TexCoord") + i + ";\n";
        else
            throw "whoops";
    }

    return ret;
}

QString GXShaderLibrary::GX_Program::generateTexCoordGetters()
{
    QString ret = "";
    for(uint32_t i = 0; i < material.texGens.size(); i++)
    {
        TexGen tg = material.texGens[i];

        if (tg.type == TexGenType::MTX2x4 || tg.type == TexGenType::SRTG)
            ret += QString("vec2 ReadTexCoord") + i + "() { return v_TexCoord" + i + ".xy; }\n";
        else if (tg.type == TexGenType::MTX3x4)
            ret += QString("vec2 ReadTexCoord") + i + "() { return v_TexCoord" + i + ".xy / v_TexCoord" + i + ".z; }\n";
        else
            throw "whoops";
    }

    return ret;
}

QString GXShaderLibrary::GX_Program::generateIndTexStageScaleN(IndTexScale_t scale)
{
    switch (scale) {
        case IndTexScale::_1:   return "1.0";
        case IndTexScale::_2:   return "1.0/2.0";
        case IndTexScale::_4:   return "1.0/4.0";
        case IndTexScale::_8:   return "1.0/8.0";
        case IndTexScale::_16:  return "1.0/16.0";
        case IndTexScale::_32:  return "1.0/32.0";
        case IndTexScale::_64:  return "1.0/64.0";
        case IndTexScale::_128: return "1.0/128.0";
        case IndTexScale::_256: return "1.0/256.0";
        default: throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateIndTexStageScale(IndTexStage stage)
{
    QString baseCoord = "ReadTexCoord" + QString::number(stage.texCoordId) + "()";
    if (stage.scaleS == IndTexScale::_1 && stage.scaleT == IndTexScale::_1)
        return baseCoord;
    else
        return baseCoord + " * vec2(" + generateIndTexStageScaleN(stage.scaleS) + ", " + generateIndTexStageScaleN(stage.scaleT) + ")";
}

QString GXShaderLibrary::GX_Program::generateTextureSample(uint32_t index, const QString& coord)
{
    return QString("texture(SAMPLER_2D(u_Texture") + index + "), " + coord + ", TextureLODBias(" + index + "))";
}

QString GXShaderLibrary::GX_Program::generateIndTexStage(uint32_t indTexStageIndex)
{
    IndTexStage stage = material.indTexStages[indTexStageIndex];
        return QString("\n\t// Indirect ") + indTexStageIndex + "\n\t" +
        "vec3 t_IndTexCoord" + indTexStageIndex + " = 255.0 * " + generateTextureSample(stage.texture, generateIndTexStageScale(stage)) + ".abg;";
}

QString GXShaderLibrary::GX_Program::generateIndTexStages()
{
    QString ret = "";
    for(uint32_t i = 0; i < material.texGens.size(); i++)
    {
        if (material.indTexStages[i].texCoordId >= material.texGens.size())
            continue;
        ret += generateIndTexStage(i);
    }

    return ret;
}

QString GXShaderLibrary::GX_Program::generateKonstColorSel(KonstColorSel_t konstColor)
{
    switch (konstColor) {
        case KonstColorSel::KCSEL_1:    return "vec3(8.0/8.0)";
        case KonstColorSel::KCSEL_7_8:  return "vec3(7.0/8.0)";
        case KonstColorSel::KCSEL_6_8:  return "vec3(6.0/8.0)";
        case KonstColorSel::KCSEL_5_8:  return "vec3(5.0/8.0)";
        case KonstColorSel::KCSEL_4_8:  return "vec3(4.0/8.0)";
        case KonstColorSel::KCSEL_3_8:  return "vec3(3.0/8.0)";
        case KonstColorSel::KCSEL_2_8:  return "vec3(2.0/8.0)";
        case KonstColorSel::KCSEL_1_8:  return "vec3(1.0/8.0)";
        case KonstColorSel::KCSEL_K0:   return "s_kColor0.rgb";
        case KonstColorSel::KCSEL_K0_R: return "s_kColor0.rrr";
        case KonstColorSel::KCSEL_K0_G: return "s_kColor0.ggg";
        case KonstColorSel::KCSEL_K0_B: return "s_kColor0.bbb";
        case KonstColorSel::KCSEL_K0_A: return "s_kColor0.aaa";
        case KonstColorSel::KCSEL_K1:   return "s_kColor1.rgb";
        case KonstColorSel::KCSEL_K1_R: return "s_kColor1.rrr";
        case KonstColorSel::KCSEL_K1_G: return "s_kColor1.ggg";
        case KonstColorSel::KCSEL_K1_B: return "s_kColor1.bbb";
        case KonstColorSel::KCSEL_K1_A: return "s_kColor1.aaa";
        case KonstColorSel::KCSEL_K2:   return "s_kColor2.rgb";
        case KonstColorSel::KCSEL_K2_R: return "s_kColor2.rrr";
        case KonstColorSel::KCSEL_K2_G: return "s_kColor2.ggg";
        case KonstColorSel::KCSEL_K2_B: return "s_kColor2.bbb";
        case KonstColorSel::KCSEL_K2_A: return "s_kColor2.aaa";
        case KonstColorSel::KCSEL_K3:   return "s_kColor3.rgb";
        case KonstColorSel::KCSEL_K3_R: return "s_kColor3.rrr";
        case KonstColorSel::KCSEL_K3_G: return "s_kColor3.ggg";
        case KonstColorSel::KCSEL_K3_B: return "s_kColor3.bbb";
        case KonstColorSel::KCSEL_K3_A: return "s_kColor3.aaa";
    }
}

QString GXShaderLibrary::GX_Program::generateKonstAlphaSel(KonstAlphaSel_t konstAlpha)
{
    switch (konstAlpha) {
        case KonstAlphaSel::KASEL_1:    return "(8.0/8.0)";
        case KonstAlphaSel::KASEL_7_8:  return "(7.0/8.0)";
        case KonstAlphaSel::KASEL_6_8:  return "(6.0/8.0)";
        case KonstAlphaSel::KASEL_5_8:  return "(5.0/8.0)";
        case KonstAlphaSel::KASEL_4_8:  return "(4.0/8.0)";
        case KonstAlphaSel::KASEL_3_8:  return "(3.0/8.0)";
        case KonstAlphaSel::KASEL_2_8:  return "(2.0/8.0)";
        case KonstAlphaSel::KASEL_1_8:  return "(1.0/8.0)";
        case KonstAlphaSel::KASEL_K0_R: return "s_kColor0.r";
        case KonstAlphaSel::KASEL_K0_G: return "s_kColor0.g";
        case KonstAlphaSel::KASEL_K0_B: return "s_kColor0.b";
        case KonstAlphaSel::KASEL_K0_A: return "s_kColor0.a";
        case KonstAlphaSel::KASEL_K1_R: return "s_kColor1.r";
        case KonstAlphaSel::KASEL_K1_G: return "s_kColor1.g";
        case KonstAlphaSel::KASEL_K1_B: return "s_kColor1.b";
        case KonstAlphaSel::KASEL_K1_A: return "s_kColor1.a";
        case KonstAlphaSel::KASEL_K2_R: return "s_kColor2.r";
        case KonstAlphaSel::KASEL_K2_G: return "s_kColor2.g";
        case KonstAlphaSel::KASEL_K2_B: return "s_kColor2.b";
        case KonstAlphaSel::KASEL_K2_A: return "s_kColor2.a";
        case KonstAlphaSel::KASEL_K3_R: return "s_kColor3.r";
        case KonstAlphaSel::KASEL_K3_G: return "s_kColor3.g";
        case KonstAlphaSel::KASEL_K3_B: return "s_kColor3.b";
        case KonstAlphaSel::KASEL_K3_A: return "s_kColor3.a";
    }
}

QString GXShaderLibrary::GX_Program::generateIndTexCoordBase(TevStage& stage)
{
    return QString("(t_IndTexCoord") + stage.indTexStage + ")";
}

QString GXShaderLibrary::GX_Program::generateAlphaBumpSelChannel(TevStage& stage)
{
    QString baseCoord = generateIndTexCoordBase(stage);
    switch (stage.indTexAlphaSel) {
        case IndTexAlphaSel::S: return baseCoord + ".x";
        case IndTexAlphaSel::T: return baseCoord + ".y";
        case IndTexAlphaSel::U: return baseCoord + ".z";
        default:
            throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateAlphaBumpSel(TevStage& stage)
{
    QString baseCoord = generateAlphaBumpSelChannel(stage);
    switch (stage.indTexFormat) {
        case IndTexFormat::_8: return "TevMask(" + baseCoord + ", 0xF8)";
        case IndTexFormat::_5: return "TevMask(" + baseCoord + ", 0xE0)";
        case IndTexFormat::_4: return "TevMask(" + baseCoord + ", 0xF0)";
        case IndTexFormat::_3: return "TevMask(" + baseCoord + ", 0xF8)";
        default:
            throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateRas(TevStage& stage)
{
    switch (stage.channelID) {
        case RasColorChannelID::COLOR0A0:     return "v_Color0";
        case RasColorChannelID::COLOR1A1:     return "v_Color1";
        case RasColorChannelID::ALPHA_BUMP:   return "vec4(" + generateAlphaBumpSel(stage) + ")";
        case RasColorChannelID::ALPHA_BUMP_N: return "vec4(" + generateAlphaBumpSel(stage) + " * (255.0/248.0))";
        case RasColorChannelID::COLOR_ZERO:   return "vec4(0, 0, 0, 0)";
        default:
            throw "whoops"; // TODO + stage.channelId;
    }
}

bool GXShaderLibrary::GX_Program::stageUsesSimpleCoords(TevStage& stage)
{
    // This is a bit of a hack. If there's no indirect stage, we use simple normalized texture coordinates;
    // this is for game renderers where injecting the texture size might be difficult.
    return stage.indTexMatrix == IndTexMtxID::OFF && !stage.indTexAddPrev;
}

QString GXShaderLibrary::GX_Program::generateTexAccess(TevStage& stage)
{
    // Skyward Sword is amazing sometimes. I hope you're happy...
    // assert(stage.texMap !== GX.TexMapID.TEXMAP_NULL);
    if (stage.texMap == TexMapID::TEXMAP_NULL)
        return "vec4(1.0, 1.0, 1.0, 1.0)";

    // If we disable textures, then return sampled white.
    if (hacks.disableTextures)
        return "vec4(1.0, 1.0, 1.0, 1.0)";

    // TODO(jstpierre): Optimize this so we don't repeat this CSE.
    QString texScale = stageUsesSimpleCoords(stage) ? "" : (QString(" * TextureInvScale(") + stage.texMap + ")");
    return generateTextureSample(stage.texMap, "t_TexCoord" + texScale);
}

char GXShaderLibrary::GX_Program::generateComponentSwizzle(SwapTable* swapTable, TevColorChan_t channel)
{
    char suffixes[] = {'r', 'g', 'b', 'a'};

    if (swapTable)
        channel = (*swapTable)[channel];

    return suffixes[channel];
}

QString GXShaderLibrary::GX_Program::generateColorSwizzle(SwapTable* swapTable, CC_t colorIn)
{
    char swapR = generateComponentSwizzle(swapTable, TevColorChan::R);
    char swapG = generateComponentSwizzle(swapTable, TevColorChan::G);
    char swapB = generateComponentSwizzle(swapTable, TevColorChan::B);
    char swapA = generateComponentSwizzle(swapTable, TevColorChan::A);

    switch (colorIn) {
        case CC::TEXC:
        case CC::RASC:
            return QString() + swapR + swapG + swapB;
        case CC::TEXA:
        case CC::RASA:
            return QString() + swapA + swapA + swapA;
        default:
            throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateColorIn(TevStage& stage, CC_t colorIn)
{
    switch (colorIn) {
        case CC::CPREV: return "t_ColorPrev.rgb";
        case CC::APREV: return "t_ColorPrev.aaa";
        case CC::C0:    return "t_Color0.rgb";
        case CC::A0:    return "t_Color0.aaa";
        case CC::C1:    return "t_Color1.rgb";
        case CC::A1:    return "t_Color1.aaa";
        case CC::C2:    return "t_Color2.rgb";
        case CC::A2:    return "t_Color2.aaa";
        case CC::TEXC:  return generateTexAccess(stage) + '.' + generateColorSwizzle(&stage.texSwapTable, colorIn);
        case CC::TEXA:  return generateTexAccess(stage) + '.' + generateColorSwizzle(&stage.texSwapTable, colorIn);
        case CC::RASC:  return "saturate(" + generateRas(stage) + '.' + generateColorSwizzle(&stage.rasSwapTable, colorIn) + ')';
        case CC::RASA:  return "saturate(" + generateRas(stage) + '.' + generateColorSwizzle(&stage.rasSwapTable, colorIn) + ')';
        case CC::ONE:   return "vec3(1)";
        case CC::HALF:  return "vec3(1.0/2.0)";
        case CC::KONST: return generateKonstColorSel(stage.konstColorSel);
        case CC::ZERO:  return "vec3(0)";
    }
}

QString GXShaderLibrary::GX_Program::generateAlphaIn(GX::TevStage& stage, GX::CA_t alphaIn)
{
    switch (alphaIn) {
        case CA::APREV: return "t_ColorPrev.a";
        case CA::A0:    return "t_Color0.a";
        case CA::A1:    return "t_Color1.a";
        case CA::A2:    return "t_Color2.a";
        case CA::TEXA:  return generateTexAccess(stage) + '.' + generateComponentSwizzle(&stage.texSwapTable, TevColorChan::A);
        case CA::RASA:  return "saturate(" + generateRas(stage) + '.' + generateComponentSwizzle(&stage.rasSwapTable, TevColorChan::A) + ')';
        case CA::KONST: return generateKonstAlphaSel(stage.konstAlphaSel);
        case CA::ZERO:  return "0.0";
        default:
            throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateTevInputs(GX::TevStage& stage)
{
    return QString() +
    "t_TevA = TevOverflow(vec4(" + generateColorIn(stage, stage.colorInA) + ", " + generateAlphaIn(stage, stage.alphaInA) + "));\n\t"
    "t_TevB = TevOverflow(vec4(" + generateColorIn(stage, stage.colorInB) + ", " + generateAlphaIn(stage, stage.alphaInB) + "));\n\t"
    "t_TevC = TevOverflow(vec4(" + generateColorIn(stage, stage.colorInC) + ", " + generateAlphaIn(stage, stage.alphaInC) + "));\n\t"
    "t_TevD = vec4(" + generateColorIn(stage, stage.colorInD) + ", " + generateAlphaIn(stage, stage.alphaInD) + ");\n";
}

QString GXShaderLibrary::GX_Program::generateTevRegister(GX::Register_t regID)
{
    switch (regID) {
        case Register::PREV: return "t_ColorPrev";
        case Register::REG0: return "t_Color0";
        case Register::REG1: return "t_Color1";
        case Register::REG2: return "t_Color2";
    }
}

QString GXShaderLibrary::GX_Program::generateTevOpBiasScaleClamp(const QString& value, GX::TevBias_t bias, GX::TevScale_t scale)
{
    QString v = value;

    if (bias == TevBias::ADDHALF)
        v = "TevBias(" + v + ", 0.5)";
    else if (bias == TevBias::SUBHALF)
        v = "TevBias(" + v + ", -0.5)";

    if (scale == TevScale::SCALE_2)
        v = "(" + v + ") * 2.0";
    else if (scale == TevScale::SCALE_4)
        v = "(" + v + ") * 4.0";
    else if (scale == TevScale::DIVIDE_2)
        v = "(" + v + ") * 0.5";

    return v;
}

QString GXShaderLibrary::GX_Program::generateTevOp(GX::TevOp_t op, GX::TevBias_t bias, GX::TevScale_t scale, const QString& a, const QString& b, const QString& c, const QString& d, const QString& zero)
{
    switch (op) {
        case TevOp::ADD:
        case TevOp::SUB:
        {
            QString neg = (op == TevOp::SUB) ? "-" : "";
            QString v = neg + "mix(" + a + ", " + b + ", " + c + ") + " + d;
            return generateTevOpBiasScaleClamp(v, bias, scale);
        }
        case TevOp::COMP_R8_GT:     return "((t_TevA.r >  t_TevB.r) ? " + c + " : " + zero + ") + " + d;
        case TevOp::COMP_R8_EQ:     return "((t_TevA.r == t_TevB.r) ? " + c + " : " + zero + ") + " + d;
        case TevOp::COMP_GR16_GT:   return "((TevPack16(t_TevA.rg) >  TevPack16(t_TevB.rg)) ? " + c + " : " + zero + ") + " + d;
        case TevOp::COMP_GR16_EQ:   return "((TevPack16(t_TevA.rg) == TevPack16(t_TevB.rg)) ? " + c + " : " + zero + ") + " + d;
        case TevOp::COMP_BGR24_GT:  return "((TevPack24(t_TevA.rgb) >  TevPack24(t_TevB.rgb)) ? " + c + " : " + zero + ") + " + d;
        case TevOp::COMP_BGR24_EQ:  return "((TevPack24(t_TevA.rgb) == TevPack24(t_TevB.rgb)) ? " + c + " : " + zero + ") + " + d;
        case TevOp::COMP_RGB8_GT:   return "(TevPerCompGT(" + a + ", " + b + ") * " + c + ") + " + d;
        case TevOp::COMP_RGB8_EQ:   return "(TevPerCompEQ(" + a + ", " + b + ") * " + c + ") + " + d;
        default:
            throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateTevOpValue(GX::TevOp_t op, GX::TevBias_t bias, GX::TevScale_t scale, bool clamp, const QString& a, const QString& b, const QString& c, const QString& d, const QString& zero)
{
    QString expr = generateTevOp(op, bias, scale, a, b, c, d, zero);

    if (clamp)
        return "saturate(" + expr + ")";
    else
        return "clamp(" + expr + ", -4.0, 4.0)";
}

QString GXShaderLibrary::GX_Program::generateColorOp(GX::TevStage& stage)
{
    QString a = "t_TevA.rgb", b = "t_TevB.rgb", c = "t_TevC.rgb", d = "t_TevD.rgb", zero = "vec3(0)";
    QString value = generateTevOpValue(stage.colorOp, stage.colorBias, stage.colorScale, stage.colorClamp, a, b, c, d, zero);
    return generateTevRegister(stage.colorRegID) + ".rgb = " + value + ";";
}

QString GXShaderLibrary::GX_Program::generateAlphaOp(GX::TevStage& stage)
{
    QString a = "t_TevA.a", b = "t_TevB.a", c = "t_TevC.a", d = "t_TevD.a", zero = "0.0";
    QString value = generateTevOpValue(stage.alphaOp, stage.alphaBias, stage.alphaScale, stage.alphaClamp, a, b, c, d, zero);
    return generateTevRegister(stage.alphaRegID) + ".a = " + value + ";";
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordWrapN(const QString& texCoord, GX::IndTexWrap_t wrap)
{
    switch (wrap) {
        case IndTexWrap::OFF:  return texCoord;
        case IndTexWrap::_0:   return "0.0";
        case IndTexWrap::_256: return "mod(" + texCoord + ", 256.0)";
        case IndTexWrap::_128: return "mod(" + texCoord + ", 128.0)";
        case IndTexWrap::_64:  return "mod(" + texCoord + ", 64.0)";
        case IndTexWrap::_32:  return "mod(" + texCoord + ", 32.0)";
        case IndTexWrap::_16:  return "mod(" + texCoord + ", 16.0)";
    }
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordWrap(GX::TevStage& stage)
{
    if (stage.texCoordID == TexCoordID::TEXCOORD_NULL || stage.texMap == TexMapID::TEXMAP_NULL)
        return "";

    uint8_t lastTexGenID = material.texGens.size() - 1;
    TexCoordID_t texGenID = stage.texCoordID;

    if (texGenID >= lastTexGenID)
        texGenID = TexCoordID_t(lastTexGenID);
    if (texGenID < 0)
        return "vec2(0.0, 0.0)";

    QString texScale = stageUsesSimpleCoords(stage) ? "" : QString(" * TextureScale(") + stage.texMap + ")";
    QString baseCoord = QString("ReadTexCoord") + texGenID + "()" + texScale;
    if (stage.indTexWrapS == IndTexWrap::OFF && stage.indTexWrapT == IndTexWrap::OFF)
        return baseCoord;
    else
        return "vec2(" + generateTevTexCoordWrapN(baseCoord + ".x", stage.indTexWrapS) + ", " + generateTevTexCoordWrapN(baseCoord + ".y", stage.indTexWrapT) + ")";
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordIndTexCoordBias(GX::TevStage& stage)
{
    QString bias = (stage.indTexFormat == IndTexFormat::_8) ? "-128.0" : "1.0";

    switch (stage.indTexBiasSel) {
        case IndTexBiasSel::NONE: return "";
        case IndTexBiasSel::S:    return " + vec3(" + bias + ", 0.0, 0.0)";
        case IndTexBiasSel::ST:   return " + vec3(" + bias + ", " + bias + ", 0.0)";
        case IndTexBiasSel::SU:   return " + vec3(" + bias + ", 0.0, " + bias + ")";
        case IndTexBiasSel::T:    return " + vec3(0.0, " + bias + ", 0.0)";
        case IndTexBiasSel::TU:   return " + vec3(0.0, " + bias + ", " + bias + ")";
        case IndTexBiasSel::U:    return " + vec3(0.0, 0.0, " + bias + ")";
        case IndTexBiasSel::STU:  return " + vec3(" + bias + ")";
    }
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordIndTexCoord(GX::TevStage& stage)
{
    QString baseCoord = generateIndTexCoordBase(stage);
    switch (stage.indTexFormat) {
        case IndTexFormat::_8: return baseCoord;
        default:
        case IndTexFormat::_5: throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordIndirectMtx(GX::TevStage& stage)
{
    QString indTexCoord = "(" + generateTevTexCoordIndTexCoord(stage) + generateTevTexCoordIndTexCoordBias(stage) + ")";

    switch (stage.indTexMatrix) {
        case IndTexMtxID::_0:  return "Mul(u_IndTexMtx[0], vec4(" + indTexCoord + ", 0.0))";
        case IndTexMtxID::_1:  return "Mul(u_IndTexMtx[1], vec4(" + indTexCoord + ", 0.0))";
        case IndTexMtxID::_2:  return "Mul(u_IndTexMtx[2], vec4(" + indTexCoord + ", 0.0))";
        case IndTexMtxID::S0:
        case IndTexMtxID::S1:
        case IndTexMtxID::S2:
            // TODO: Although u_IndTexMtx is ignored, the result is still scaled by the scale_exp argument passed into GXSetIndTexMtx.
            // This assumes scale_exp is 0.
            return QString("(ReadTexCoord") + stage.texCoordID + "() * " + indTexCoord + ".xx)";
        case IndTexMtxID::T0:
        case IndTexMtxID::T1:
        case IndTexMtxID::T2:
            // TODO: Although u_IndTexMtx is ignored, the result is still scaled by the scale_exp argument passed into GXSetIndTexMtx.
            // This assumes scale_exp is 0.
            return QString("(ReadTexCoord") + stage.texCoordID + "() * " + indTexCoord + ".yy)";
        // TODO(jstpierre): These other options. BossBakkunPlanet.arc uses them.
        default:
            std::cerr << "Unimplemented indTexMatrix mode: " << stage.indTexMatrix;
            return "" + indTexCoord + ".xy";
    }
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordIndirectTranslation(GX::TevStage& stage)
{
    if (stage.indTexMatrix != IndTexMtxID::OFF && stage.indTexStage < material.indTexStages.size())
        return generateTevTexCoordIndirectMtx(stage);
    else
        return "";
}

QString GXShaderLibrary::GX_Program::generateTevTexCoordIndirect(GX::TevStage& stage)
{
    QString baseCoord = generateTevTexCoordWrap(stage);
    QString indCoord = generateTevTexCoordIndirectTranslation(stage);

    if (!baseCoord.isEmpty() && !indCoord.isEmpty())
        return baseCoord + " + "+ indCoord;
    else if (!baseCoord.isEmpty())
        return baseCoord;
    else
        return indCoord;
}

QString GXShaderLibrary::GX_Program::generateTevTexCoord(GX::TevStage& stage)
{
    QString finalCoord = generateTevTexCoordIndirect(stage);
    if (!finalCoord.isEmpty()) {
        if (stage.indTexAddPrev) {
            return "t_TexCoord += " + finalCoord + ";";
        } else {
            return "t_TexCoord = " + finalCoord + ";";
        }
    } else {
        return "";
    }
}

QString GXShaderLibrary::GX_Program::generateTevStage(uint32_t tevStageIndex)
{
    TevStage& stage = material.tevStages[tevStageIndex];

    return QString() +
"\n\t// TEV Stage " + tevStageIndex +
"\n\t" + generateTevTexCoord(stage) + R"(
    // Color Combine
    // colorIn: )" + stage.colorInA + ' ' + stage.colorInB + ' ' + stage.colorInC + ' ' + stage.colorInD + "  colorOp: " + stage.colorOp + " colorBias: " + stage.colorBias + " colorScale: " + stage.colorScale + " colorClamp: " + stage.colorClamp + " colorRegId: " + stage.colorRegID +
"\n\t// alphaIn: " + stage.alphaInA + ' ' + stage.alphaInB + ' ' + stage.alphaInC + ' ' + stage.alphaInD + "  alphaOp: " + stage.alphaOp + " alphaBias: " + stage.alphaBias + " alphaScale: " + stage.alphaScale + " alphaClamp: " + stage.alphaClamp + " alphaRegId: " + stage.alphaRegID +
"\n\t// texCoordId: " + stage.texCoordID + " texMap: " + stage.texMap + " channelId: " + stage.channelID +
"\n\t" + generateTevInputs(stage) +
"\n\t" + generateColorOp(stage) +
"\n\t" + generateAlphaOp(stage);
}

QString GXShaderLibrary::GX_Program::generateTevStages()
{
    QString ret = "";
    for(uint32_t i = 0; i < material.tevStages.size(); i++)
        ret += generateTevStage(i) + "\n";

    return ret;
}

QString GXShaderLibrary::GX_Program::generateTevStagesLastMinuteFixup()
{
    std::vector<TevStage> tevStages = material.tevStages;
    // Despite having a destination register, the output of the last stage
    // is what gets output from the color combinations...
    TevStage& lastTevStage = tevStages[tevStages.size() - 1];
    QString colorReg = generateTevRegister(lastTevStage.colorRegID);
    QString alphaReg = generateTevRegister(lastTevStage.alphaRegID);

    if (colorReg == alphaReg)
        return "\nvec4 t_TevOutput = " + colorReg + ";";
    else
        return "\nvec4 t_TevOutput = vec4(" + colorReg + ".rgb, " + alphaReg + ".a);";
}


// TODO continue at https://github.com/magcius/noclip.website/blob/master/src/gx/gx_material.ts#L1165
