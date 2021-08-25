#include "pch.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
    m_device = nullptr;
    m_deviceContext = nullptr;
    m_disposed = false;
    m_maxParticles = -1;
    m_firstRun = true;
    m_drawing = true;
    m_age = 0.f;

    particleData.gameTime = 0.f;
    particleData.deltaTime = 0.f;
    particleData.camPosition = XMFLOAT3(0.f, 0.f, 0.f);
    particleData.emitPosition = XMFLOAT3(0.f, 0.f, 0.f);
    particleData.emitDirection = XMFLOAT3(0.f, 1.f, 0.f); // Up

    m_texArraySRV = nullptr;
    m_randomTexSRV = nullptr;
}

void ParticleSystem::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::wstring texArrayPath, int maxParticles)
{
    m_device = device;
    m_deviceContext = deviceContext;
    m_maxParticles = maxParticles;

    // Textures
    
    // - Particle Texture 
    m_texArraySRV = ResourceHandler::getInstance().getTexture(texArrayPath.c_str());
    
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
    device->CreateTexture1D(&texDesc, &texData, &randomTexture);

    // - - Shader Resource View
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    srvDesc.Texture1D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture1D.MostDetailedMip = 0;

    HRESULT hr = device->CreateShaderResourceView(randomTexture, &srvDesc, &m_randomTexSRV);
    randomTexture->Release();

    // Shaders
    ShaderFiles files;
    files.vs = L"ParticleDrawVS.hlsl";
    files.gs = L"ParticleDrawGS.hlsl";
    files.ps = L"ParticleDrawPS.hlsl";
    m_drawShaders.initialize(m_device, deviceContext, files, LayoutType::PARTICLE, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    files.vs = L"ParticleSoVS.hlsl";
    files.gs = L"ParticleSoGS.hlsl";
    files.ps = L"";
    m_streamOutputShaders.initialize(m_device, deviceContext, files, LayoutType::PARTICLE, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, true);

    // Vertex Buffers
    VertexParticle particle;
    particle.age = 0.f;
    particle.type = 0;

    std::vector<VertexParticle> particles(m_maxParticles, particle);

    m_initVertexBuffer.initialize(m_device, m_deviceContext, &particle, BufferType::VERTEX, 1, false, true);
    m_drawVertexBuffer.initialize(m_device, m_deviceContext, particles.data(), BufferType::VERTEX, m_maxParticles, false, true);
    m_streamOutVertexBuffer.initialize(m_device, m_deviceContext, particles.data(), BufferType::VERTEX, m_maxParticles, false, true);
}

void ParticleSystem::reset()
{
    m_firstRun = true;
    m_drawing = true;
    m_age = 0;
}

void ParticleSystem::update(float dt, float gameTime, Camera &camera)
{
    particleData.gameTime = gameTime;
    particleData.deltaTime = dt;
    m_age += dt;
    particleData.camPosition = camera.getCameraPositionF3();
    particleData.viewProjectionMatrix = camera.getViewMatrix() * camera.getViewMatrix();
}

void ParticleSystem::render()
{
    // Set Textures
    m_deviceContext->GSSetShaderResources(0, 1, &m_randomTexSRV);
    m_deviceContext->PSSetShaderResources(0, 1, &m_texArraySRV);
    
    // Set Shaders
    m_streamOutputShaders.setShaders();

    // bind the input vertex buffer for the stream-out technique
    // use the _initVB when _firstRun = true
    UINT offset = 0;
    if (m_firstRun)
        m_deviceContext->IASetVertexBuffers(0, 1, m_initVertexBuffer.GetAddressOf(), m_initVertexBuffer.getStridePointer(), &offset);
    else
    {
        if(m_drawing)
            m_deviceContext->IASetVertexBuffers(0, 1, m_drawVertexBuffer.GetAddressOf(), m_drawVertexBuffer.getStridePointer(), &offset);
        else
            m_deviceContext->IASetVertexBuffers(0, 1, m_streamOutVertexBuffer.GetAddressOf(), m_streamOutVertexBuffer.getStridePointer(), &offset);
    }

    // bind the stream-out vertex buffer
    if (m_drawing)
        m_deviceContext->SOSetTargets(1, m_streamOutVertexBuffer.GetAddressOf(), &offset);
    else
        m_deviceContext->SOSetTargets(1, m_drawVertexBuffer.GetAddressOf(), &offset);

    // Bind Constant Buffer
    m_deviceContext->GSSetConstantBuffers(0, 1, particleCBuffer.GetAddressOf());

    // draw the particles using the stream-out technique, which will update the particles positions
    // and output the resulting particles to the stream-out buffer
    if (m_firstRun)
    {
        m_deviceContext->Draw(1, 0);
        m_firstRun = false;
    }
    else
    {
        // the _drawVB buffer was populated by the Stream-out technique, so we don't
        // know how many vertices are contained within it.  Direct3D keeps track of this
        // internally, however, and we can use DrawAuto to draw everything in the buffer.
        m_deviceContext->DrawAuto();
    }
    // Disable stream-out
    m_deviceContext->SOSetTargets(0, nullptr, &offset);

    // ping-pong the stream-out and draw buffers, since we will now want to draw the vertices
    // populated into the buffer that was bound to stream-out
    m_drawing = !m_drawing;

    // draw the particles using the draw technique that will transform the points to lines/quads
    if (m_drawing)
        m_deviceContext->IASetVertexBuffers(0, 1, m_drawVertexBuffer.GetAddressOf(), m_drawVertexBuffer.getStridePointer(), &offset);
    else
        m_deviceContext->IASetVertexBuffers(0, 1, m_streamOutVertexBuffer.GetAddressOf(), m_streamOutVertexBuffer.getStridePointer(), &offset);
 
    // Draw pass stuff
    m_drawShaders.setShaders();

    // Draw Streamed Out Particles
    m_deviceContext->DrawAuto();
}
