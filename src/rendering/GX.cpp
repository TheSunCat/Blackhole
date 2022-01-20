#include "rendering/GX.h"

void GX::autoOptimizeMaterial(Material& mat)
{
    if(!mat.hasPostTexMtxBlock)
        mat.hasPostTexMtxBlock = autoOptimizeMaterialHasPostTexMtxBlock(mat);
    if(!mat.hasLightsBlock)
        mat.hasLightsBlock = autoOptimizeMaterialHasLightsBlock(mat);
    if(!mat.hasFogBlock)
        mat.hasFogBlock = autoOptimizeMaterialHasFogBlock(mat);
}

bool GX::autoOptimizeMaterialHasPostTexMtxBlock(Material& mat)
{
    for(TexGen& texGen : mat.texGens)
    {
        if(texGen.postMatrix != GX::PostTexGenMatrix::PTIDENTITY)
            return true;
    }

    return false;
}

bool GX::autoOptimizeMaterialHasLightsBlock(Material& mat)
{
    for(LightChannelControl& lightChannel : mat.lightChannels)
    {
        if(lightChannel.colorChannel.lightingEnabled && lightChannel.colorChannel.litMask != 0)
            return true;
        if(lightChannel.alphaChannel.lightingEnabled && lightChannel.alphaChannel.litMask != 0)
            return true;
    }

    return false;
}

bool GX::autoOptimizeMaterialHasFogBlock(Material& mat)
{
    return mat.ropInfo.fogType != GX::FogType::NONE;
}
