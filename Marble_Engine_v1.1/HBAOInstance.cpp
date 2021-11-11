#include "pch.h"
#include "HBAOInstance.h"

void HBAOInstance::initHBAOTexture(ID3D11Device* device, int width, int height)
{
    // Texture
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&textureDesc, NULL, &m_texture.rtt);
    assert(SUCCEEDED(hr) && "Error, render target texture could not be created!");

    // Render Rarget View
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    hr = device->CreateRenderTargetView(m_texture.rtt, &renderTargetViewDesc, &m_texture.rtv);
    assert(SUCCEEDED(hr) && "Error, render target view could not be created!");

    // Shader Resource View
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_texture.rtt, &srvDesc, &m_texture.srv);
    assert(SUCCEEDED(hr) && "Error, shader resource view could not be created!");

    // Unordered Access View
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = textureDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    hr = device->CreateUnorderedAccessView(m_texture.rtt, &uavDesc, &m_texture.uav);
    assert(SUCCEEDED(hr) && "Error, unordered access view could not be created!");
}

void HBAOInstance::initRandomTexture(ID3D11Device* device)
{
    HRESULT hr;

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
    std::vector<XMFLOAT4> data(len);

    /*for (size_t i = 0; i < len; i += 3) {
        data[i] = XMFLOAT4(frand(-1.0f, 1.0f), frand(-1.0f, 1.0f), 0.0f, 0.0f);

        XMVECTOR vec = XMVector3Normalize(XMLoadFloat4(&m_HBAOData.sampleDirections[i]));
        XMStoreFloat4(&m_HBAOData.sampleDirections[i], vec);
    }*/

    std::vector<float> offsets1;
    std::vector<float> offsets2;

    for (size_t i = 0; i < len; ++i)
    {
        offsets1.push_back((float)i / len);
        offsets2.push_back((float)i / len - 1);
    }

    unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(offsets1.begin(), offsets1.end(), std::default_random_engine(seed));
    seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(offsets2.begin(), offsets2.end(), std::default_random_engine(seed));

    int i = 0;
    for (int y = 0; y < ditherHeight; ++y)
    {
        for (int x = 0; x < ditherWidth; ++x)
        {
            float r = offsets1[i];
            float g = offsets2[i];

            data[i] = XMFLOAT4(r, g, 1.f, 1.f);
            ++i;
        }
    }

    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = data.data();
    subData.SysMemPitch = texDesc.Width * sizeof(XMFLOAT4);
    subData.SysMemSlicePitch = subData.SysMemPitch * texDesc.Height;

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

void HBAOInstance::initSamplerStates(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC samplerStateDesc;
    samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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

    m_deviceContext->PSSetSamplers(3, 1, m_depthNormalSamplerState.GetAddressOf());
    m_deviceContext->PSSetSamplers(4, 1, m_randomSamplerState.GetAddressOf());
}

void HBAOInstance::initSampleDirections()
{
    //// 8 cube corners
    //m_HBAOData.sampleDirections[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.f);
    //m_HBAOData.sampleDirections[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.f);
    //m_HBAOData.sampleDirections[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.f);
    //m_HBAOData.sampleDirections[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.f);
    //m_HBAOData.sampleDirections[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.f);
    //m_HBAOData.sampleDirections[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.f);
    //m_HBAOData.sampleDirections[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.f);
    //m_HBAOData.sampleDirections[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.f);

    //// 6 centers of cube faces
    //m_HBAOData.sampleDirections[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.f);
    //m_HBAOData.sampleDirections[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.f);
    //m_HBAOData.sampleDirections[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.f);
    //m_HBAOData.sampleDirections[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.f);
    //m_HBAOData.sampleDirections[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.f);
    //m_HBAOData.sampleDirections[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.f);

    //for (int i = 0; i < HBAO_SAMPLE_DIRECTIONS; ++i) {
    //    float s = frand(0.25f, 1.0f);
    //    XMVECTOR vec = XMVector3Normalize(XMLoadFloat4(&m_HBAOData.sampleDirections[i]));
    //    vec *= s;
    //    XMStoreFloat4(&m_HBAOData.sampleDirections[i], vec);
    //}

    for (int i = 0; i < HBAO_SAMPLE_DIRECTIONS; ++i)
    {
        float angle = (float)i / HBAO_SAMPLE_DIRECTIONS * 2.f * XM_PI;
        float r = std::cos(angle);
        float g = std::sin(angle);

        m_HBAOData.sampleDirections[i].x = r;
        m_HBAOData.sampleDirections[i].y = g;


        //m_HBAOData.sampleDirections[i] = XMFLOAT4(
            /*frand(-1.0f, 1.0f),
            frand(-1.0f, 1.0f),
            frand(0.0f, 1.0f), 0.f);*/

            /*XMVECTOR vec = XMVector3Normalize(XMLoadFloat4(&m_HBAOData.sampleDirections[i]));
            float scale = float(i) / float(HBAO_SAMPLE_DIRECTIONS);
            scale = lerpF(0.1f, 1.0f, scale * scale);
            vec *= scale;
            XMStoreFloat4(&m_HBAOData.sampleDirections[i], vec);*/
    }
}

HBAOInstance::HBAOInstance() {}

HBAOInstance::~HBAOInstance() {}

void HBAOInstance::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height, float farZ, float fov, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
    // DeviceConstext
    m_deviceContext = deviceContext;

    // Shader
    ShaderFiles shaderFiles;
    shaderFiles.vs = L"HBAO_VS.hlsl";
    shaderFiles.ps = L"HBAO_PS.hlsl";
    m_HBAOPassShaders.initialize(device, deviceContext, shaderFiles, LayoutType::POS_NOR_TEX);

    // HBAO Texture
    initHBAOTexture(device, width, height);

    // Random Texture
    initRandomTexture(device);

    // Sampler State Setup
    initSamplerStates(device);

    // Constant Buffers
    // - HBAO Buffer Sample Directions
    initSampleDirections();

    // - Dither Scale for Random Texture
    m_HBAOData.ditherScale = XMFLOAT2((float)width / 4.f, (float)height / 4.f);
    m_HBAOBuffer.initialize(device, deviceContext, &m_HBAOData, BufferType::CONSTANT);

    // - HBAO Matrix Buffer
    m_invProjectionMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix));
    m_HBAOMatrixBuffer.initialize(device, deviceContext, &m_invProjectionMatrix, BufferType::CONSTANT);

    // - SSAO Camera Buffer
    updateBuffers(width, height, farZ, fov, viewMatrix, projectionMatrix);
    m_HBAOCameraBuffer.initialize(device, deviceContext, &m_HBAOCameraData, BufferType::CONSTANT);
}

void HBAOInstance::updateBuffers(int width, int height, float farZ, float fov, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
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

    m_HBAOCameraData.viewToTexMatrix = XMMatrixTranspose(pt);*/
    m_HBAOCameraData.projectionMatrix = XMMatrixTranspose(projectionMatrix);
    m_HBAOCameraData.invProjectionMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix));
    m_HBAOCameraData.viewMatrix = XMMatrixTranspose(viewMatrix);
    float halfHeight = farZ * tanf(0.5f * fov);
    float halfWidth = aspect * halfHeight;

    XMFLOAT3 frustumCorners[8];
    XMFLOAT3 frontFrustumVectors[4];
    BoundingFrustum frustum;
    BoundingFrustum::CreateFromMatrix(frustum, projectionMatrix);
    frustum.GetCorners(frustumCorners);
    for (size_t i = 0; i < 4; i++)
    {
        frontFrustumVectors[i] = XMFLOAT3(frustumCorners[4 + i].x / farZ, frustumCorners[4 + i].y / farZ, frustumCorners[4 + i].z / farZ);
    }

    /*
    m_HBAOFrustumData.viewFrustumVectors[0] = XMVectorSet(frustumCorners[7].x, frustumCorners[7].y, frustumCorners[7].z, 1.f) / farZ;
    m_HBAOFrustumData.viewFrustumVectors[1] = XMVectorSet(frustumCorners[4].x, frustumCorners[4].y, frustumCorners[4].z, 1.f) / farZ;
    m_HBAOFrustumData.viewFrustumVectors[2] = XMVectorSet(frustumCorners[5].x, frustumCorners[5].y, frustumCorners[5].z, 1.f) / farZ;
    m_HBAOFrustumData.viewFrustumVectors[3] = XMVectorSet(frustumCorners[6].x, frustumCorners[6].y, frustumCorners[6].z, 1.f) / farZ;

    m_HBAOCameraData.viewFrustumVectors[0] = XMVectorSet(frustumCorners[7].x, frustumCorners[7].y, frustumCorners[7].z, 1.f) / farZ;
    m_HBAOCameraData.viewFrustumVectors[1] = XMVectorSet(frustumCorners[4].x, frustumCorners[4].y, frustumCorners[4].z, 1.f) / farZ;
    m_HBAOCameraData.viewFrustumVectors[2] = XMVectorSet(frustumCorners[5].x, frustumCorners[5].y, frustumCorners[5].z, 1.f) / farZ;
    m_HBAOCameraData.viewFrustumVectors[3] = XMVectorSet(frustumCorners[6].x, frustumCorners[6].y, frustumCorners[6].z, 1.f) / farZ;
    */

    m_invProjectionMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, projectionMatrix));

    m_HBAOCameraData.viewFrustumVectors[0] = XMFLOAT4(-halfWidth / farZ, -halfHeight / farZ, 1.f, 1.f);
    m_HBAOCameraData.viewFrustumVectors[1] = XMFLOAT4(-halfWidth / farZ, +halfHeight / farZ, 1.f, 1.f);
    m_HBAOCameraData.viewFrustumVectors[2] = XMFLOAT4(+halfWidth / farZ, +halfHeight / farZ, 1.f, 1.f);
    m_HBAOCameraData.viewFrustumVectors[3] = XMFLOAT4(+halfWidth / farZ, -halfHeight / farZ, 1.f, 1.f);

    m_HBAOCameraData.renderTargetResolution = XMFLOAT2((float)(width), (float)(height));
}

void HBAOInstance::updateViewMatrix(XMMATRIX viewMatrix)
{
    m_HBAOCameraData.viewMatrix = XMMatrixTranspose(viewMatrix);

    PS_HBAO_CAMERA_BUFFER* HBAOCameraData = new PS_HBAO_CAMERA_BUFFER(m_HBAOCameraData);
    m_HBAOCameraBuffer.update(&HBAOCameraData);
}

void HBAOInstance::updateShaders()
{
    m_HBAOPassShaders.updateShaders();
}

void HBAOInstance::render()
{
    // Set Render Target
    m_deviceContext->OMSetRenderTargets(1, &m_texture.rtv, nullptr);

    // Shader
    m_HBAOPassShaders.setShaders();

    // Constant Buffers
    m_deviceContext->VSSetConstantBuffers(0, 1, m_HBAOCameraBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_HBAOCameraBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(1, 1, m_HBAOBuffer.GetAddressOf());

    // Dither Texture
    m_deviceContext->PSSetShaderResources(2, 1, m_ditherTextureSRV.GetAddressOf());

    // Draw Fullscreen Quad
    m_deviceContext->Draw(6, 0);

    // Reset
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, nullptr);
}
