#ifndef HBAOINSTANCE_H
#define HBAOMAPINSTANCE_H

#include "Shaders.h"

const int HBAO_SAMPLE_DIRECTIONS = 16;

struct PS_HBAO_BUFFER
{
    XMFLOAT4 sampleDirections[HBAO_SAMPLE_DIRECTIONS];
    XMFLOAT2 ditherScale;
    XMFLOAT2 pad;
};

struct PS_HBAO_CAMERA_BUFFER
{
    //XMMATRIX viewToTexMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX invProjectionMatrix;
    XMMATRIX viewMatrix;
    XMFLOAT4 viewFrustumVectors[4];
    XMFLOAT2 renderTargetResolution;
    XMFLOAT2 pad;
};

class HBAOInstance
{
private:
    // Device
    ID3D11DeviceContext* m_deviceContext;

    // Constant Buffers
    PS_HBAO_BUFFER m_HBAOData;
    PS_HBAO_CAMERA_BUFFER m_HBAOCameraData;
    XMMATRIX m_invProjectionMatrix;

    Buffer<PS_HBAO_BUFFER> m_HBAOBuffer;
    Buffer<PS_HBAO_CAMERA_BUFFER> m_HBAOCameraBuffer;
    Buffer<XMMATRIX> m_HBAOMatrixBuffer;

    // Texture
    RenderTexture m_texture;
    ID3D11RenderTargetView* m_renderTargetNullptr = nullptr;

    // Shader
    Shaders m_HBAOPassShaders;

    // Sampler
    ComPtr< ID3D11SamplerState > m_depthNormalSamplerState;
    ComPtr< ID3D11SamplerState > m_randomSamplerState;

    // Dither Texture
    ComPtr< ID3D11Texture2D > m_ditherTexture;
    ComPtr< ID3D11ShaderResourceView > m_ditherTextureSRV;

    void initHBAOTexture(ID3D11Device* device, int width, int height);
    void initRandomTexture(ID3D11Device* device);
    void initSamplerStates(ID3D11Device* device);
    void initSampleDirections();

public:
    HBAOInstance();
    ~HBAOInstance();

    void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height, float farZ, float fov, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);

    RenderTexture& getAORenderTexture() { return m_texture; }

    void updateBuffers(int width, int height, float farZ, float fov, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix);
    void updateViewMatrix(XMMATRIX viewMatrix);
    void updateShaders();

    void UIRenderDitherTextureWindow()
    {
        ImGui::Begin("Dither Texture HBAO");
        ImGui::Image(m_ditherTextureSRV.Get(), ImVec2(100.f, 100.f));
        ImGui::End();
    }

    void render();
};

#endif // !HBAOMAPINSTANCE_H