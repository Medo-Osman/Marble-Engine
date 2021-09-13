#include "pch.h"
#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include "RenderObject.h"

const UINT POINT_LIGHT = 0;
const UINT SPOT_LIGHT = 1;
const UINT DIRECTIONAL_LIGHT = 2;

class LightManager
{
private:
    // Device
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;

    // Lights
    PS_LIGHT_BUFFER m_lightData;

    // Constant Buffer
    Buffer<PS_LIGHT_BUFFER> m_lightBuffer;

    // Render Objects for Point and Spot Lights
    std::vector<std::pair<UINT, RenderObject*>> m_renderObjects;
    XMMATRIX* m_viewMatrix;
    XMMATRIX* m_projectionMatrix;

public:
	LightManager()
    {
        m_device = nullptr;
        m_deviceContext = nullptr;
        m_viewMatrix = nullptr;
        m_projectionMatrix = nullptr;
        m_lightData.nrOfLights = 0;
    }
	~LightManager() {}

    // Initialize
    void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX* viewMatrix, XMMATRIX* projectionMatrix)
    {
        m_device = device;
        m_deviceContext = deviceContext;

        m_viewMatrix = viewMatrix;
        m_projectionMatrix = projectionMatrix;

        m_lightBuffer.initialize(device, deviceContext, &m_lightData, BufferType::CONSTANT);
    }

    // Getters
    ID3D11Buffer* Get() const { return m_lightBuffer.Get(); }
    ID3D11Buffer* const* GetAddressOf() const { return m_lightBuffer.GetAddressOf(); }
    int const getNrOfLights() const { return m_lightData.nrOfLights; }

    // Update
    bool addLight(Light newLight)
    {
        if (m_lightData.nrOfLights >= LIGHT_CAP)
            return false;
        m_lightData.lights[m_lightData.nrOfLights] = newLight;
        m_lightData.nrOfLights++;

        if (newLight.type == POINT_LIGHT || newLight.type == SPOT_LIGHT)
        {
            // Add Render Object
            m_renderObjects.push_back(std::make_pair( m_lightData.nrOfLights - 1, new RenderObject() ));
            m_renderObjects.back().second->initialize(m_device, m_deviceContext, (int)m_renderObjects.size(), "sphere.obj");
            TexturePaths textures;
            textures.diffusePath = L"circle_pattern.png";
            m_renderObjects.back().second->setTextures(textures);
            PS_MATERIAL_BUFFER material;
            material.ambient = newLight.color;
            material.diffuse = newLight.color;
            material.specular = newLight.color;
            material.emissive = newLight.color;
            m_renderObjects.back().second->setMaterial(material);
        }

        return true;
    }

    void removeLight(int id)
    {
        std::vector<Light> lights;
        for (size_t i = 0; i < m_lightData.nrOfLights; i++)
            lights.push_back(m_lightData.lights[i]);
        
        m_lightData.nrOfLights--;
        lights.erase(lights.begin() + id);
        if (m_lightData.lights[id].type == POINT_LIGHT || m_lightData.lights[id].type == SPOT_LIGHT)
        {
            for (size_t j = 0; j < m_renderObjects.size(); j++)
            {
                if (m_renderObjects[j].first == id)
                {
                    delete m_renderObjects[j].second;
                    m_renderObjects.erase(m_renderObjects.begin() + j);
                    break;
                }
            }
        }

        for (size_t i = 0; i < m_lightData.nrOfLights; i++)
        {
            m_lightData.lights[i] = lights[i];
        }
        
        update();
    }

    void updateLight(Light* light, int id)
    {
        m_lightData.lights[id] = *light;
        update();
    }

    void enableLight(UINT index)
    {
        m_lightData.lights[index].enabled = true;
    }
    void disableLight(UINT index)
    {
        m_lightData.lights[index].enabled = false;
    }

    void update()
    {
        PS_LIGHT_BUFFER* lightData = new PS_LIGHT_BUFFER(m_lightData);
        m_lightBuffer.update(&lightData);
    }

    void renderLightIndicators()
    {
        XMMATRIX viewMatrix = *m_viewMatrix;
        XMMATRIX projMatrix = *m_projectionMatrix;
        //for (size_t i = 0; i < m_renderObjects.size(); i++)
        //{
        //    //XMMATRIX worldMatrix = XMMatrixIdentity();
        //    XMMATRIX worldMatrix = XMMatrixScalingFromVector(XMVectorSet(.01f, .01f, .01f, 0.f));
        //    if (m_lightData.lights[m_renderObjects[i].first].type == SPOT_LIGHT)
        //        worldMatrix *= XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&m_lightData.lights[m_renderObjects[i].first].direction));

        //    worldMatrix *= XMMATRIX(XMMatrixTranslationFromVector(XMLoadFloat4(&m_lightData.lights[m_renderObjects[i].first].position)));

        //    m_renderObjects[i].second->updateWCPBuffer(worldMatrix, viewProjMatrix);
        //    m_renderObjects[i].second->render();
        //}
        for (size_t i = 0; i < m_lightData.nrOfLights; i++)
        {
            if (m_lightData.lights[i].type == POINT_LIGHT || m_lightData.lights[i].type == SPOT_LIGHT)
            {
                TexturePaths textures;
                textures.diffusePath = L"circle_pattern.png";
                m_renderObjects.back().second->setTextures(textures);
                PS_MATERIAL_BUFFER material;
                XMFLOAT4 ligthColor = m_lightData.lights[i].color;
                material.ambient = ligthColor;
                material.diffuse = ligthColor;
                material.specular = ligthColor;
                material.emissive = ligthColor;
                m_renderObjects.back().second->setMaterial(material);

                XMMATRIX worldMatrix = XMMatrixScalingFromVector(XMVectorSet(.01f, .01f, .01f, 0.f));
                if (m_lightData.lights[i].type == SPOT_LIGHT)
                    worldMatrix *= XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&m_lightData.lights[i].direction));

                worldMatrix *= XMMATRIX(XMMatrixTranslationFromVector(XMLoadFloat4(&m_lightData.lights[i].position)));


                m_renderObjects.back().second->updateWCPBuffer(worldMatrix, viewMatrix, projMatrix);
                m_renderObjects.back().second->render(true);
            }
        }
    }
};

#endif // !LIGHTMANAGER_H