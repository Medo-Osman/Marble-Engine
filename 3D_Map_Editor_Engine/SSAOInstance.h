#include "pch.h"
#ifndef SSAOINSTANCE_H
#define SSAOINSTANCE_H

const int SSAO_SAMPLE_DIRECTIONS = 14;

struct PS_SSAO_BUFFER
{
    XMFLOAT4 sampleDirections[SSAO_SAMPLE_DIRECTIONS];
    XMFLOAT2 ditherScale;
    XMFLOAT2 pad;
};

struct PS_SSAO_CAMERA_BUFFER
{
    XMMATRIX viewToTexMatrix;
    XMMATRIX projectionMatrix;
    XMMATRIX viewMatrix;
};

class SSAOInstance
{
private:
    // Device
    ID3D11DeviceContext* m_deviceContext;

    // Constant Buffers
    PS_SSAO_BUFFER m_SSAOData;
    PS_SSAO_CAMERA_BUFFER m_SSAOCameraData;
    XMMATRIX m_invProjectionMatrix;

    Buffer<PS_SSAO_BUFFER> m_SSAOBuffer;
    Buffer<PS_SSAO_CAMERA_BUFFER> m_SSAOCameraBuffer;
    Buffer<XMMATRIX> m_SSAOMatrixBuffer;

    // Shader
    Shaders m_SSAOPassShaders;

    // Sampler
    ComPtr< ID3D11SamplerState > m_depthNormalSamplerState;
    ComPtr< ID3D11SamplerState > m_randomSamplerState;

    // Dither Texture
    ComPtr< ID3D11Texture2D > m_ditherTexture;
    ComPtr< ID3D11ShaderResourceView > m_ditherTextureSRV;

    // Functions
    void initRandomTexture(ID3D11Device* device)
    {
        // Dither Texture
        //int ditherWidth = 4;
        //int ditherHeight = 4;

        //D3D11_TEXTURE2D_DESC texDesc;
        //texDesc.Width = ditherWidth;
        //texDesc.Height = ditherHeight;
        //texDesc.MipLevels = 1;
        //texDesc.ArraySize = 1;
        //texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        //texDesc.SampleDesc.Count = 1;
        //texDesc.SampleDesc.Quality = 0;
        //texDesc.Usage = D3D11_USAGE_DEFAULT;
        //texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        //texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        //texDesc.MiscFlags = 0;

        //int len = ditherWidth * ditherHeight;
        //std::vector<float> data(len);
        //std::vector<float> offsets1;
        //std::vector<float> offsets2;

        //for (size_t i = 0; i < len; ++i)
        //{
        //    offsets1.push_back((float)i / len);
        //    offsets2.push_back((float)i / len);
        //}

        //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        //std::shuffle(offsets1.begin(), offsets1.end(), std::default_random_engine(seed));
        //seed = std::chrono::system_clock::now().time_since_epoch().count();
        //std::shuffle(offsets2.begin(), offsets2.end(), std::default_random_engine(seed));

        //int i = 0;
        //int j = 0;
        //for (int y = 0; y < ditherHeight; ++y)
        //{
        //    for (int x = 0; x < ditherWidth; ++x)
        //    {
        //        float r = offsets1[i];
        //        float g = offsets2[i];
        //        ++i;

        //        data[j] = std::round(r * 0xff);
        //        data[j + 1] = std::round(g * 0xff);
        //        data[j + 2] = 0;
        //        data[j + 3] = 1;
        //    }
        //}

        //D3D11_SUBRESOURCE_DATA subData;
        //subData.pSysMem = data.data();
        //subData.SysMemPitch = texDesc.Width * sizeof(float) * 4;
        ////subData.SysMemSlicePitch = texDesc.Width * texDesc.Height * sizeof(float) * 4;

        HRESULT hr;
        int dimensions = 256;
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width = dimensions;
        texDesc.Height = dimensions;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DYNAMIC;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        texDesc.MiscFlags = 0;

        XMVECTOR* randomColors = new XMVECTOR[dimensions * dimensions];
        for (int i = 0; i < dimensions; i++)
        {
            for (int j = 0; j < dimensions; j++)
                randomColors[i * dimensions + j] = XMVectorSet(((float)(rand()) / (float)RAND_MAX), ((float)(rand()) / (float)RAND_MAX), ((float)(rand()) / (float)RAND_MAX), ((float)(rand()) / (float)RAND_MAX));
        }

        D3D11_SUBRESOURCE_DATA subData;
        subData.pSysMem = randomColors;
        subData.SysMemPitch = texDesc.Width * sizeof(XMVECTOR);

        hr = device->CreateTexture2D(&texDesc, &subData, m_ditherTexture.GetAddressOf());
        assert(SUCCEEDED(hr) && "Error, failed to create dither texture!");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(m_ditherTexture.Get(), &srvDesc, m_ditherTextureSRV.GetAddressOf());
        assert(SUCCEEDED(hr) && "Error, failed to create dither texture shader resource view!");
    }
    void initSamplerStates(ID3D11Device* device)
    {
        D3D11_SAMPLER_DESC samplerStateDesc;
        samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerStateDesc.MinLOD = (-FLT_MAX);
        samplerStateDesc.MaxLOD = (FLT_MAX);
        samplerStateDesc.MipLODBias = 0.0f;
        samplerStateDesc.MaxAnisotropy = 1;
        samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerStateDesc.BorderColor[0] = 1.f;
        samplerStateDesc.BorderColor[1] = 1.f;
        samplerStateDesc.BorderColor[2] = 1.f;
        samplerStateDesc.BorderColor[3] = 1.f;

        HRESULT hr = device->CreateSamplerState(&samplerStateDesc, &m_randomSamplerState);
        assert(SUCCEEDED(hr) && "Error, failed to create depth normal sampler state!");

        samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerStateDesc.BorderColor[0] = 1.f;
        samplerStateDesc.BorderColor[1] = 1.f;
        samplerStateDesc.BorderColor[2] = 1.f;
        samplerStateDesc.BorderColor[3] = 1.f;

        hr = device->CreateSamplerState(&samplerStateDesc, &m_depthNormalSamplerState);
        assert(SUCCEEDED(hr) && "Error, failed to create random sampler state!");

        m_deviceContext->PSSetSamplers(2, 1, m_depthNormalSamplerState.GetAddressOf());
        m_deviceContext->PSSetSamplers(3, 1, m_randomSamplerState.GetAddressOf());
    }
    void initSampleDirections()
    {
        // 8 cube corners
        m_SSAOData.sampleDirections[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.f);
        m_SSAOData.sampleDirections[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.f);
        m_SSAOData.sampleDirections[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.f);
        m_SSAOData.sampleDirections[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.f);
        m_SSAOData.sampleDirections[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.f);
        m_SSAOData.sampleDirections[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.f);
        m_SSAOData.sampleDirections[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.f);
        m_SSAOData.sampleDirections[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.f);

        // 6 centers of cube faces
        m_SSAOData.sampleDirections[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.f);
        m_SSAOData.sampleDirections[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.f);
        m_SSAOData.sampleDirections[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.f);
        m_SSAOData.sampleDirections[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.f);
        m_SSAOData.sampleDirections[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.f);
        m_SSAOData.sampleDirections[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.f);

        for (int i = 0; i < 14; ++i) {
            float s = frand(0.25f, 1.f);
            XMVECTOR vec = XMVector3Normalize(XMLoadFloat4(&m_SSAOData.sampleDirections[i]));
            vec *= s;
            XMStoreFloat4(&m_SSAOData.sampleDirections[i], vec);
        }
    }

public:
    SSAOInstance() {}
	~SSAOInstance() {}

    void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height, float farZ, float fov, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
    {
        // DeviceConstext
        m_deviceContext = deviceContext;

        // Shader
        ShaderFiles shaderFiles;
        shaderFiles.vs = L"SSAO_VS.hlsl";
        shaderFiles.ps = L"SSAO_PS.hlsl";
        m_SSAOPassShaders.initialize(device, deviceContext, shaderFiles, LayoutType::POS);

        // Random Texture
        initRandomTexture(device);

        // Sampler State Setup
        initSamplerStates(device);

        // Constant Buffers
        // - SSAO Buffer Sample Directions
        initSampleDirections();

        // - Dither Scale for Random Texture
        m_SSAOData.ditherScale = XMFLOAT2((float)width / 4, (float)height / 4);
        m_SSAOBuffer.initialize(device, deviceContext, &m_SSAOData, BufferType::CONSTANT);
        
        // - SSAO Matrix Buffer
        m_invProjectionMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix));
        m_SSAOMatrixBuffer.initialize(device, deviceContext, &m_invProjectionMatrix, BufferType::CONSTANT);

        // - SSAO Camera Buffer
        updateBuffers(width, height, farZ, fov, viewMatrix, projectionMatrix);
        m_SSAOCameraBuffer.initialize(device, deviceContext, &m_SSAOCameraData, BufferType::CONSTANT);
    }

    void updateBuffers(int width, int height, float farZ, float fov, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
    {
        // SSAO Data
        float aspect = (float)(width) / (float)(height);
        
        // SSAO Camera and Frustum Data
        XMMATRIX T(
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f);
        XMMATRIX P = projectionMatrix;
        XMMATRIX pt = XMMatrixMultiply(P, T);

        m_SSAOCameraData.viewToTexMatrix = XMMatrixTranspose(pt);
        m_SSAOCameraData.projectionMatrix = XMMatrixTranspose(projectionMatrix);
        m_SSAOCameraData.viewMatrix = XMMatrixTranspose(viewMatrix);
        float halfHeight = farZ * tanf(0.5f * fov);
        float halfWidth = aspect * halfHeight;
    }
    void updateViewMatrix(XMMATRIX viewMatrix)
    {
        m_SSAOCameraData.viewMatrix = XMMatrixTranspose(viewMatrix);

        PS_SSAO_CAMERA_BUFFER* SSAOCameraData = new PS_SSAO_CAMERA_BUFFER(m_SSAOCameraData);
        m_SSAOCameraBuffer.update(&SSAOCameraData);
    }
    void updateShaders()
    {
        m_SSAOPassShaders.updateShaders();
    }

    void UIRenderDitherTextureWindow()
    {
        ImGui::Begin("Dither Texture");
        ImGui::Image(m_ditherTextureSRV.Get(), ImVec2(100.f, 100.f));
        ImGui::End();
    }

    void render()
    {
        // Shader
        m_SSAOPassShaders.setShaders();

        // Constant Buffers
        m_deviceContext->VSSetConstantBuffers(0, 1, m_SSAOMatrixBuffer.GetAddressOf());
        m_deviceContext->PSSetConstantBuffers(0, 1, m_SSAOCameraBuffer.GetAddressOf());
        m_deviceContext->PSSetConstantBuffers(1, 1, m_SSAOBuffer.GetAddressOf());

        // Dither Texture
        m_deviceContext->PSSetShaderResources(2, 1, m_ditherTextureSRV.GetAddressOf());

        // Draw Fullscreen Quad
        m_deviceContext->Draw(6, 0);
    }
};

#endif // !SSAOINSTANCE_H