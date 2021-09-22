#include "pch.h"
#ifndef GBUFFER_H
#define GBUFFER_H

enum GBufferType { ALBEDO_METALLIC, NORMAL_ROUGNESS, EMISSIVE_SHADOWMASK, AMBIENT_OCCLUSION, DEPTH, GB_NUM};

struct GBuffer
{
    RenderTexture renderTextures[GBufferType::GB_NUM];
};

#endif // !GBUFFER_H