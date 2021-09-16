#include "pch.h"
#ifndef HBAOINSTANCE_H
#define HBAOMAPINSTANCE_H

const int HBAO_SAMPLE_DIRECTIONS = 16;

struct PS_HBAO_BUFFER
{
    float sampleDirections[HBAO_SAMPLE_DIRECTIONS * 2];
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

struct VS_HBAO_BUFFER
{
    XMFLOAT4 viewFrustumVectors[4];
    XMMATRIX invProjectionMatrix;
};

class HBAOInstance
{
private:
    // Device
    ID3D11DeviceContext* m_deviceContext;

    // Constant Buffers
    PS_HBAO_BUFFER m_hbaoData;
    PS_HBAO_CAMERA_BUFFER m_hbaoCameraData;
    VS_HBAO_BUFFER m_hbaoFrustumData;

    Buffer<PS_HBAO_BUFFER> m_hbaoBuffer;
    Buffer<PS_HBAO_CAMERA_BUFFER> m_hbaoCameraBuffer;
    Buffer<VS_HBAO_BUFFER> m_hbaoFrustumBuffer;

    // Fullscreen Quad
    Buffer<VertexPosNormTex> m_fullscreenQuadVertexBuffer;

    // Shader
    Shaders m_HBAOPassShaders;

    // Sampler
    ComPtr< ID3D11SamplerState > m_depthNormalSamplerState;
    ComPtr< ID3D11SamplerState > m_randomSamplerState;

    // Dither Texture
    ComPtr< ID3D11Texture2D > m_ditherTexture;
    ComPtr< ID3D11ShaderResourceView > m_ditherTextureSRV;

public:
    HBAOInstance() {}
	~HBAOInstance() {}

    void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height, float farZ, float fov, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
    {
        // DeviceConstext
        m_deviceContext = deviceContext;

        // Shader
        ShaderFiles shaderFiles;
        shaderFiles.vs = L"HBAO_VS.hlsl";
        shaderFiles.ps = L"HBAO_PS.hlsl";
        m_HBAOPassShaders.initialize(device, deviceContext, shaderFiles, LayoutType::POS_NOR_TEX);

        // Sampler State Setup
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
        samplerStateDesc.BorderColor[0] = 0.f;
        samplerStateDesc.BorderColor[1] = 0.f;
        samplerStateDesc.BorderColor[2] = 0.f;
        samplerStateDesc.BorderColor[3] = 0.f;

        hr = device->CreateSamplerState(&samplerStateDesc, &m_depthNormalSamplerState);
        assert(SUCCEEDED(hr) && "Error, failed to create random sampler state!");

        m_deviceContext->PSSetSamplers(2, 1, m_depthNormalSamplerState.GetAddressOf());
        m_deviceContext->PSSetSamplers(3, 1, m_randomSamplerState.GetAddressOf());

        // Constant Buffers
        // - HBAO Buffer Sample Directions
        int j = 0;
        float alpha = 2.0 * XM_PI / HBAO_SAMPLE_DIRECTIONS;
        for (int i = 0; i < HBAO_SAMPLE_DIRECTIONS; ++i)
        {
            float angle = alpha * i;
            float r = std::cos(angle);
            float g = std::sin(angle);

            /*float angle = (float)i / 16.f * 2.f * XM_PI;
            float r = std::cos(angle) * .5f + .5f;
            float g = std::sin(angle) * .5f + .5f;*/
            m_hbaoData.sampleDirections[j++] = r;
            m_hbaoData.sampleDirections[j++] = g;
        }
        // 8 cube corners
        //m_hbaoData.sampleDirections[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.f);
        //m_hbaoData.sampleDirections[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.f);
        //m_hbaoData.sampleDirections[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.f);
        //m_hbaoData.sampleDirections[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.f);
        //m_hbaoData.sampleDirections[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.f);
        //m_hbaoData.sampleDirections[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.f);
        //m_hbaoData.sampleDirections[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.f);
        //m_hbaoData.sampleDirections[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.f);

        //// 6 centers of cube faces
        //m_hbaoData.sampleDirections[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.f);
        //m_hbaoData.sampleDirections[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.f);
        //m_hbaoData.sampleDirections[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.f);
        //m_hbaoData.sampleDirections[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.f);
        //m_hbaoData.sampleDirections[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.f);
        //m_hbaoData.sampleDirections[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.f);

        //for (int i = 0; i < 14; ++i) {
        //    float s = frand(0.25f, 0.7f);
        //    XMVECTOR vec = XMVector3Normalize(XMLoadFloat4(&m_hbaoData.sampleDirections[i]));
        //    vec *= s;
        //    XMStoreFloat4(&m_hbaoData.sampleDirections[i], vec);
        //}
        m_hbaoData.ditherScale = XMFLOAT2((float)width / 4, (float)height / 4);
        m_hbaoBuffer.initialize(device, deviceContext, &m_hbaoData, BufferType::CONSTANT);
        
        // - HBAO Camera and HBAO Frustum Buffers
        updateBuffers(width, height, farZ, fov, viewMatrix, projectionMatrix);
        m_hbaoCameraBuffer.initialize(device, deviceContext, &m_hbaoCameraData, BufferType::CONSTANT);
        m_hbaoFrustumBuffer.initialize(device, deviceContext, &m_hbaoFrustumData, BufferType::CONSTANT);

        // Dither Texture
        int ditherWidth = 4;
        int ditherHeight = 4;

        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width = ditherWidth;
        texDesc.Height = ditherHeight;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        texDesc.MiscFlags = 0;

        int len = ditherWidth * ditherHeight;
        std::vector<float> data(len);
        std::vector<float> offsets1;
        std::vector<float> offsets2;

        for (int i = 0; i < len; ++i)
        {
            offsets1.push_back((float)i / len);
            offsets2.push_back((float)i / len);
        }

        unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(offsets1.begin(), offsets1.end(), std::default_random_engine(seed));
        seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(offsets2.begin(), offsets2.end(), std::default_random_engine(seed));

        int i = 0;
        j = 0;
        for (int y = 0; y < ditherHeight; ++y)
        {
            for (int x = 0; x < ditherWidth; ++x) 
            {
                float r = offsets1[i];
                float g = offsets2[i];
                ++i;

                data[j] = std::round(r * 0xff);
                data[j + 1] = std::round(g * 0xff);
                data[j + 2] = 0;
                data[j + 3] = 1;
            }
        }

        D3D11_SUBRESOURCE_DATA subData;
        subData.pSysMem = data.data();
        subData.SysMemPitch = texDesc.Width * sizeof(float) * 4;
        //subData.SysMemSlicePitch = texDesc.Width * texDesc.Height * sizeof(float) * 4;

        hr = device->CreateTexture2D(&texDesc, &subData, m_ditherTexture.GetAddressOf());
        assert(SUCCEEDED(hr) && "Error, failed to create dither texture!");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(m_ditherTexture.Get(), &srvDesc, m_ditherTextureSRV.GetAddressOf());
        assert(SUCCEEDED(hr) && "Error, failed to create dither texture shader resource view!");

        // Fullscreen Quad Vertices
        std::vector<VertexPosNormTex> fullScreenQuad;

        fullScreenQuad.push_back({ XMFLOAT3(-1.f, -1.f, 0.f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f, 1.f) });
        fullScreenQuad.push_back({ XMFLOAT3(-1.f, 1.f, 0.f), XMFLOAT3(1.f, 0.f, 0.f), XMFLOAT2(0.f, 0.f) });
        fullScreenQuad.push_back({ XMFLOAT3(1.f, 1.f, 0.f), XMFLOAT3(2.f, 0.f, 0.f), XMFLOAT2(1.f, 0.f) });
        fullScreenQuad.push_back({ XMFLOAT3(-1.f, -1.f, 0.f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT2(0.f, 1.f) });
        fullScreenQuad.push_back({ XMFLOAT3(1.f, 1.f, 0.f), XMFLOAT3(2.f, 0.f, 0.f), XMFLOAT2(1.f, 0.f) });
        fullScreenQuad.push_back({ XMFLOAT3(1.f, -1.f, 0.f), XMFLOAT3(3.f, 0.f, 0.f), XMFLOAT2(1.f, 1.f) });

        m_fullscreenQuadVertexBuffer.initialize(device, deviceContext, fullScreenQuad.data(), BufferType::VERTEX, (int)fullScreenQuad.size());
    }

    void updateBuffers(int width, int height, float farZ, float fov, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
    {
        // HBAO Data
        float aspect = (float)(width) / (float)(height);
        
        // HBAO Camera and Frustum Data
        /*XMMATRIX T(
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f);
        XMMATRIX P = projectionMatrix;
        XMMATRIX pt = XMMatrixMultiply(P, T);

        m_hbaoCameraData.viewToTexMatrix = XMMatrixTranspose(pt);*/
        m_hbaoCameraData.projectionMatrix = XMMatrixTranspose(projectionMatrix);
        m_hbaoCameraData.invProjectionMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix));
        m_hbaoCameraData.viewMatrix = XMMatrixTranspose(viewMatrix);
        float halfHeight = farZ * tanf(0.5f * fov);
        float halfWidth = aspect * halfHeight;

        XMFLOAT3 frustumCorners[8];
        XMFLOAT3 frontFrustumVectors[4];
        BoundingFrustum frustum;
        BoundingFrustum::CreateFromMatrix(frustum, projectionMatrix);
        frustum.GetCorners(frustumCorners);
        for (size_t i = 0; i < 4; i++)
        {
            frontFrustumVectors[i] = XMFLOAT3(frustumCorners[4+i].x / farZ, frustumCorners[4 + i].y / farZ, frustumCorners[4 + i].z / farZ);
        }

        /*
        m_hbaoFrustumData.viewFrustumVectors[0] = XMVectorSet(frustumCorners[7].x, frustumCorners[7].y, frustumCorners[7].z, 1.f) / farZ;
        m_hbaoFrustumData.viewFrustumVectors[1] = XMVectorSet(frustumCorners[4].x, frustumCorners[4].y, frustumCorners[4].z, 1.f) / farZ;
        m_hbaoFrustumData.viewFrustumVectors[2] = XMVectorSet(frustumCorners[5].x, frustumCorners[5].y, frustumCorners[5].z, 1.f) / farZ;
        m_hbaoFrustumData.viewFrustumVectors[3] = XMVectorSet(frustumCorners[6].x, frustumCorners[6].y, frustumCorners[6].z, 1.f) / farZ;

        m_hbaoCameraData.viewFrustumVectors[0] = XMVectorSet(frustumCorners[7].x, frustumCorners[7].y, frustumCorners[7].z, 1.f) / farZ;
        m_hbaoCameraData.viewFrustumVectors[1] = XMVectorSet(frustumCorners[4].x, frustumCorners[4].y, frustumCorners[4].z, 1.f) / farZ;
        m_hbaoCameraData.viewFrustumVectors[2] = XMVectorSet(frustumCorners[5].x, frustumCorners[5].y, frustumCorners[5].z, 1.f) / farZ;
        m_hbaoCameraData.viewFrustumVectors[3] = XMVectorSet(frustumCorners[6].x, frustumCorners[6].y, frustumCorners[6].z, 1.f) / farZ;
        */

        m_hbaoFrustumData.viewFrustumVectors[0] = XMFLOAT4(-halfWidth / farZ, -halfHeight / farZ, 1.f, 1.f);
        m_hbaoFrustumData.viewFrustumVectors[1] = XMFLOAT4(-halfWidth / farZ, +halfHeight / farZ, 1.f, 1.f);
        m_hbaoFrustumData.viewFrustumVectors[2] = XMFLOAT4(+halfWidth / farZ, +halfHeight / farZ, 1.f, 1.f);
        m_hbaoFrustumData.viewFrustumVectors[3] = XMFLOAT4(+halfWidth / farZ, -halfHeight / farZ, 1.f, 1.f);
        
        m_hbaoFrustumData.invProjectionMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix));

        m_hbaoCameraData.viewFrustumVectors[0] = XMFLOAT4(-halfWidth / farZ, -halfHeight / farZ, 1.f, 1.f);
        m_hbaoCameraData.viewFrustumVectors[1] = XMFLOAT4(-halfWidth / farZ, +halfHeight / farZ, 1.f, 1.f);
        m_hbaoCameraData.viewFrustumVectors[2] = XMFLOAT4(+halfWidth / farZ, +halfHeight / farZ, 1.f, 1.f);
        m_hbaoCameraData.viewFrustumVectors[3] = XMFLOAT4(+halfWidth / farZ, -halfHeight / farZ, 1.f, 1.f);
        
        m_hbaoCameraData.renderTargetResolution = XMFLOAT2((float)(width), (float)(height));


    }
    void updateViewMatrix(XMMATRIX viewMatrix)
    {
        m_hbaoCameraData.viewMatrix = XMMatrixTranspose(viewMatrix);

        PS_HBAO_CAMERA_BUFFER* hbaoCameraData = new PS_HBAO_CAMERA_BUFFER(m_hbaoCameraData);
        m_hbaoCameraBuffer.update(&hbaoCameraData);
    }
    void updateShaders()
    {
        m_HBAOPassShaders.updateShaders();
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
        m_HBAOPassShaders.setShaders();

        // Vertex Buffer
        UINT offset = 0;
        m_deviceContext->IASetVertexBuffers(0, 1, m_fullscreenQuadVertexBuffer.GetAddressOf(), m_fullscreenQuadVertexBuffer.getStridePointer(), &offset);

        // Constant Buffers
        m_deviceContext->VSSetConstantBuffers(0, 1, m_hbaoFrustumBuffer.GetAddressOf());
        m_deviceContext->PSSetConstantBuffers(0, 1, m_hbaoCameraBuffer.GetAddressOf());
        m_deviceContext->PSSetConstantBuffers(1, 1, m_hbaoBuffer.GetAddressOf());

        // Dither Texture
        m_deviceContext->PSSetShaderResources(2, 1, m_ditherTextureSRV.GetAddressOf());

        // Draw Fullscreen Quad
        m_deviceContext->Draw(6, 0);
    }
};

#endif // !HBAOMAPINSTANCE_H