#include "pch.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
    m_device = nullptr;
    m_deviceContext = nullptr;
    m_disposed = false;
    m_maxParticles = -1;
    m_firstRun = true;
    m_age = 0.f;

    m_particleData.gameTime = 0.f;
    m_particleData.deltaTime = 0.f;
    m_particleData.camPosition = XMFLOAT3(0.f, 0.f, 0.f);
    m_particleData.emitPosition = XMFLOAT3(0.f, 0.f, 0.f);

    m_texture1SRV = nullptr;
    m_randomTexSRV = nullptr;
}

void ParticleSystem::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::wstring texArrayPath, int maxParticles, PARTICLE_STYLE styleData)
{
    m_device = device;
    m_deviceContext = deviceContext;
    m_maxParticles = maxParticles;

    // Textures
    
    // - Particle Texture 
    m_texture1SRV = ResourceHandler::getInstance().getTexture(texArrayPath.c_str());

    // - Random Values Texture
    // - - Texture
    XMVECTOR randomValues[1024];
    for (int i = 0; i < 1024; i++)
        randomValues[i] = (XMVectorSet(frand(-1.0f, 1.0f), frand(-1.0f, 1.0f), frand(-1.0f, 1.0f), frand(-1.0f, 1.0f)));
    
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

    D3D11_SUBRESOURCE_DATA texData;
    texData.pSysMem = randomValues;
    texData.SysMemPitch = 1024 * sizeof(XMVECTOR);

    ID3D11Texture1D* randomTexture;
    HRESULT hr = m_device->CreateTexture1D(&texDesc, &texData, &randomTexture);
    assert(SUCCEEDED(hr) && "Error, failed to create random texture!");

    // - - Shader Resource View
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    srvDesc.Texture1D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture1D.MostDetailedMip = 0;

    hr = m_device->CreateShaderResourceView(randomTexture, &srvDesc, &m_randomTexSRV);
    assert(SUCCEEDED(hr) && "Error, failed to create random texture SRV!");
    randomTexture->Release();

    // - Noise Texture
    m_noiseTexSRV = ResourceHandler::getInstance().getTexture(L"noise_clouds.png");

    // Shaders
    ShaderFiles files;
    files.vs = L"ParticleSoVS.hlsl";
    files.gs = L"ParticleSoGS.hlsl";
    m_streamOutputShaders.initialize(m_device, m_deviceContext, files, LayoutType::PARTICLE, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, true);

    files.vs = L"ParticleDrawVS.hlsl";
    files.gs = L"ParticleDrawGS.hlsl";
    files.ps = L"ParticleDrawPS.hlsl";
    m_drawShaders.initialize(m_device, m_deviceContext, files, LayoutType::PARTICLE, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // Vertex Buffers
    VertexParticle particle;
    particle.position = XMFLOAT3(0, 2, 0);
    particle.velocity = XMFLOAT3(0, 0, 0);
    particle.size = XMFLOAT2(3.f, 3.f);
    particle.rotation = 0.f;
    particle.age = 0.f;
    particle.type = ParticleType::EMITTER;
    particle.maxId = 0;

    m_initVertexBuffer.initialize(m_device, m_deviceContext, &particle, BufferType::VERTEX, 1, false, true);
    m_drawVertexBuffer.initialize(m_device, m_deviceContext, nullptr, BufferType::VERTEX, m_maxParticles, false, true);
    m_streamOutVertexBuffer.initialize(m_device, m_deviceContext, nullptr, BufferType::VERTEX, m_maxParticles, false, true);

    // Style
    m_particleStyleData = styleData;

    // Data
    m_particleData.maxParticles = (float)m_maxParticles;

    // Constant Buffer
    m_particleCBuffer.initialize(m_device, m_deviceContext, &m_particleData, BufferType::CONSTANT);
    m_particleStyleCBuffer.initialize(m_device, m_deviceContext, &m_particleStyleData, BufferType::CONSTANT);
}

void ParticleSystem::setEmitPosition(XMFLOAT3 newPosition)
{
    m_particleData.emitPosition = newPosition;
    m_particleCBuffer.update(&m_particleData);
}

void ParticleSystem::reset()
{
    m_firstRun = true;
    m_age = 0;
}

void ParticleSystem::updateShaders()
{
    m_streamOutputShaders.updateShaders();
    m_drawShaders.updateShaders();
    reset();
}

void ParticleSystem::update(double dt, float gameTime, Camera &camera)
{
    m_particleData.camPosition = camera.getCameraPositionF3();

    m_particleData.gameTime = gameTime;
    m_particleData.deltaTime = (float)dt;
    m_age += (float)dt;
    m_particleData.viewMatrix = XMMatrixTranspose(camera.getViewMatrix());
    m_particleData.projMatrix = XMMatrixTranspose(camera.getProjectionMatrix());
    m_particleCBuffer.update(&m_particleData);
}

void ParticleSystem::generateParticles()
{
    // Texture
    m_deviceContext->GSSetShaderResources(0, 1, &m_randomTexSRV);
    m_deviceContext->PSSetShaderResources(0, 1, &m_texture1SRV);
    m_deviceContext->PSSetShaderResources(1, 1, &m_noiseTexSRV);

    // Constant Buffer
    m_deviceContext->GSSetConstantBuffers(0, 1, m_particleCBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_particleCBuffer.GetAddressOf());

    m_deviceContext->VSSetConstantBuffers(2, 1, m_particleStyleCBuffer.GetAddressOf());
    m_deviceContext->GSSetConstantBuffers(1, 1, m_particleStyleCBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(1, 1, m_particleStyleCBuffer.GetAddressOf());

    // Shaders
    m_streamOutputShaders.setShaders();

    // Vertex Buffer
    UINT offset = 0;
    if (m_firstRun)
        m_deviceContext->IASetVertexBuffers(0, 1, m_initVertexBuffer.GetAddressOf(), m_initVertexBuffer.getStridePointer(), &offset);
    else
        m_deviceContext->IASetVertexBuffers(0, 1, m_drawVertexBuffer.GetAddressOf(), m_drawVertexBuffer.getStridePointer(), &offset);

    // Bind Stream-Out Vertex Buffer
    m_deviceContext->SOSetTargets(1, m_streamOutVertexBuffer.GetAddressOf(), &offset);
    
    // Draw
    if (m_firstRun)
    {
        m_deviceContext->Draw(1, 0);
        m_firstRun = false;
    }
    else
        m_deviceContext->DrawAuto();
}

void ParticleSystem::renderParticles() // generateParticles needs to be called before
{
    // Disable stream-out
    UINT offset[] = { 0 };
    ID3D11Buffer* buffer[1] = { 0 };
    m_deviceContext->SOSetTargets(1, buffer, offset);

    // Ping-Pong Vertex Buffers
    std::swap(m_drawVertexBuffer, m_streamOutVertexBuffer);

    // Vertex Buffer
    m_deviceContext->IASetVertexBuffers(0, 1, m_drawVertexBuffer.GetAddressOf(), m_drawVertexBuffer.getStridePointer(), offset);
    
    // Shaders
    m_drawShaders.setShaders();

    // Draw
    m_deviceContext->DrawAuto();
}
