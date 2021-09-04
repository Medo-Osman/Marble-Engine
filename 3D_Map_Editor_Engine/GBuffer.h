#include "pch.h"
#ifndef GBUFFER_H
#define GBUFFER_H


struct RenderTarget
{
    ID3D11Texture2D* rtt;
    ID3D11RenderTargetView* rtv;
    ID3D11ShaderResourceView* srv;
    DXGI_FORMAT format;

    RenderTarget()
    {
        this->rtt = nullptr;
        this->rtv = nullptr;
        this->srv = nullptr;
        this->format = DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    ~RenderTarget()
    {
        if (this->rtt)
            this->rtt->Release();
        if (this->rtv)
            this->rtv->Release();
        if (this->srv)
            this->srv->Release();
    }
};

enum GBufferType { ALBEDO_METALLIC, NORMAL_ROUGNESS, EMISSIVE_SHADOWMASK, AMBIENT_OCCLUSION, DEPTH, GB_NUM};

struct GBuffer
{
    RenderTarget renderTargets[GBufferType::GB_NUM];
};

#endif // !GBUFFER_H
