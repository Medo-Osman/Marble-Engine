#ifndef TEXTUREHELPER_H
#define TEXTUREHELPER_H

struct RenderTexture
{
    ID3D11Texture2D* rtt;
    ID3D11RenderTargetView* rtv;
    ID3D11ShaderResourceView* srv;
    ID3D11UnorderedAccessView* uav;
    DXGI_FORMAT format;

    RenderTexture()
    {
        this->rtt = nullptr;
        this->rtv = nullptr;
        this->srv = nullptr;
        this->uav = nullptr;
        this->format = DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    ~RenderTexture()
    {
        if (this->rtt)
            this->rtt->Release();
        if (this->rtv)
            this->rtv->Release();
        if (this->srv)
            this->srv->Release();
        if (this->uav)
            this->uav->Release();
    }
};

#endif // !TEXTUREHELPER_H