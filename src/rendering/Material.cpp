#include "rendering/Material.h"

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
    //    return `vec4(1.0, 1.0, 1.0, 1.0)`;

    switch (chan.matColorSource) {
        case GX::ColorSrc::VTX: return QString("a_Color") + i;
        case GX::ColorSrc::REG: return QString("u_ColorMatReg[") + i + "]";
    }

    return "";
}

QString GXShaderLibrary::GX_Program::generateAmbientSource(ColorChannelControl chan, uint32_t i) {
    //if (this.hacks !== null && this.hacks.disableVertexColors && chan.ambColorSource === GX.ColorSrc.VTX)
    //    return `vec4(1.0, 1.0, 1.0, 1.0)`;

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
        return R"(
    t_Attenuation = ${cosAttn} / ${distAttn};)";
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
            generateLightAccum = R"(
    t_LightAccum = ${ambSource};)";

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
QString GXShaderLibrary::GX_Program::generateMulPntMatrixStatic(TexGenMatrix pnt, const QString& src, const QString& funcName) {
    if (pnt == GX::TexGenMatrix::IDENTITY) {
        return src + ".xyz";
    } else if (pnt >= GX::TexGenMatrix::TEXMTX0) {
        QString texMtxIdx = QString::number((uint32_t(pnt) - uint32_t(GX::TexGenMatrix::TEXMTX0)) / 3);
        return funcName + "(u_TexMtx[" + texMtxIdx + "], " + src + ")";
    } else if (pnt >= GX::TexGenMatrix::PNMTX0) {
        QString pnMtxIdx = QString::number((uint32_t(pnt) - uint32_t(GX::TexGenMatrix::PNMTX0)) / 3);
        return funcName + "(u_PosMtx[" + pnMtxIdx + "], " + src + ")";
    } else {
        throw "whoops";
    }
}

// Output is a vec3, src is a vec4.
QString GXShaderLibrary::GX_Program::generateMulPntMatrixDynamic(const QString& attrStr, const QString& src, const QString& funcName) {
    return funcName + "(GetPosTexMatrix(" + attrStr + "), " + src + ")";
}

QString GXShaderLibrary::GX_Program::generateTexMtxIdxAttr(TexCoordID index) {
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
QString GXShaderLibrary::GX_Program::generateTexGenSource(GX::TexGenSrc src)
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

QString GXShaderLibrary::GX_Program::generatePostTexGenMatrixMult(GX::TexGen texCoordGen, const QString& src)
{
    if (texCoordGen.postMatrix == PostTexGenMatrix::PTIDENTITY) {
        return src + ".xyz";
    } else if (texCoordGen.postMatrix >= PostTexGenMatrix::PTTEXMTX0) {
        QString texMtxIdx = QString::number((uint32_t(texCoordGen.postMatrix) - uint32_t(PostTexGenMatrix::PTTEXMTX0)) / 3);
        return "Mul(u_PostTexMtx[" + texMtxIdx + "], " + src + ")";
    } else {
        throw "whoops";
    }
}

QString GXShaderLibrary::GX_Program::generateTexGenMatrixMult(uint32_t texCoordGenIndex, const QString& src)
{
    if (material.useTexMtxIdx[texCoordGenIndex]) {
        QString attrStr = generateTexMtxIdxAttr(TexCoordID(texCoordGenIndex));
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
// TexGen ") + i + " Type: " + uint32_t(tg.type) + " Source: " + uint32_t(tg.source) + " Matrix: " + uint32_t(tg.matrix) + "\n\
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

QString GXShaderLibrary::GX_Program::generateIndTexStageScaleN(GX::IndTexScale scale)
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

QString GXShaderLibrary::GX_Program::generateIndTexStageScale(GX::IndTexStage stage)
{
    QString baseCoord = "ReadTexCoord" + QString::number(uint32_t(stage.texCoordId)) + "()";
    if (stage.scaleS == IndTexScale::_1 && stage.scaleT == IndTexScale::_1)
        return baseCoord;
    else
        return baseCoord + " * vec2(" + generateIndTexStageScaleN(stage.scaleS) + ", " + generateIndTexStageScaleN(stage.scaleT) + ")";
}



// TODO continue at https://github.com/magcius/noclip.website/blob/master/src/gx/gx_material.ts#L700
