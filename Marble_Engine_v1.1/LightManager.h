#include "pch.h"
#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include "RenderObject.h"

enum LightType { POINT_LIGHT, SPOT_LIGHT, DIRECTIONAL_LIGHT, NONE };
static const char* LightTypeNames[] = { "Point", "Spot", "Directional"};

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
    std::vector<RenderObject> m_renderObjects;
    static const UINT POINT_MESH = 0;
    static const UINT SPOT_MESH = 1;
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

        // Add Render Objects
        // - Sphere
        m_renderObjects.push_back(RenderObject());
        m_renderObjects.back().initialize(m_device, m_deviceContext, (int)m_renderObjects.size(), "sphere.obj");
        TexturePaths textures;
        textures.diffusePath = L"DefaultWhite.jpg";
        m_renderObjects.back().setTextures(textures);
        // - Cone
        m_renderObjects.push_back(RenderObject());
        m_renderObjects.back().initialize(m_device, m_deviceContext, (int)m_renderObjects.size(), "cone.glb");
        m_renderObjects.back().setTextures(textures);
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

        return true;
    }

    void removeLight(int id)
    {
        std::vector<Light> lights;
        for (size_t i = 0; i < m_lightData.nrOfLights; i++)
            lights.push_back(m_lightData.lights[i]);

        m_lightData.nrOfLights--;
        lights.erase(lights.begin() + id);
        /*if (m_lightData.lights[id].type == POINT_LIGHT || m_lightData.lights[id].type == SPOT_LIGHT)
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
        }*/

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

    void enviormentDiffContributionUI()
    {
        ImGui::PushItemWidth(-90.f);
        if (ImGui::SliderFloat("Diffuse##Diff", &m_lightData.enviormentDiffContribution, 0.f, 1.f))
            update();
        ImGui::PopItemWidth();
    }
    void enviormentSpecContributionUI()
    {
        ImGui::PushItemWidth(-90.f);
        if (ImGui::SliderFloat("Specular##Spec", &m_lightData.enviormentSpecContribution, 0.f, 1.f))
            update();
        ImGui::PopItemWidth();
    }

    void setVolumetricSunScattering(bool toggle)
    {
        m_lightData.volumetricSunScattering = toggle;
        update();
    }
    void setFog(bool toggle)
    {
        m_lightData.fog = toggle;
        update();
    }
    void setProceduralSky(bool toggle)
    {
        m_lightData.procederualSky = toggle;
        update();
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
            if (m_lightData.lights[i].type == POINT_LIGHT)
            {
                PS_MATERIAL_BUFFER material;
                XMFLOAT4 col = XMFLOAT4(m_lightData.lights[i].color.x * 4.f, m_lightData.lights[i].color.y * 4.f, m_lightData.lights[i].color.z * 4.f, 1.f);
                material.ambient = col;
                material.diffuse = col;
                material.specular = XMFLOAT4(0,0,0,0);
                material.emissive = col;
                m_renderObjects[POINT_MESH].setMaterial(material);

                XMMATRIX worldMatrix = XMMatrixScaling(.03f, .03f, .03f);

                worldMatrix *= XMMATRIX(XMMatrixTranslationFromVector(XMLoadFloat4(&m_lightData.lights[i].position)));

                m_renderObjects[POINT_MESH].updateWCPBuffer(worldMatrix, viewMatrix, projMatrix);
                m_renderObjects[POINT_MESH].render(true);
            }
            else if (m_lightData.lights[i].type == SPOT_LIGHT)
            {
                PS_MATERIAL_BUFFER material;
                XMFLOAT4 col = XMFLOAT4(m_lightData.lights[i].color.x * 4.f, m_lightData.lights[i].color.y * 4.f, m_lightData.lights[i].color.z * 4.f, 1.f);
                material.ambient = col;
                material.diffuse = col;
                material.specular = XMFLOAT4(0,0,0,0);
                material.emissive = col;
                m_renderObjects[SPOT_MESH].setMaterial(material);

                XMMATRIX worldMatrix = XMMatrixIdentity();
                worldMatrix *= lookAtMatrix(XMLoadFloat4(&m_lightData.lights[i].position), XMVector3Normalize(XMVectorSetW(XMLoadFloat3(&m_lightData.lights[i].direction), 0.f)), XMVectorSet(0,1,0,0));
                
                m_renderObjects[SPOT_MESH].updateWCPBuffer(worldMatrix, viewMatrix, projMatrix);
                m_renderObjects[SPOT_MESH].render(true);
            }
            else if (m_lightData.lights[i].type == DIRECTIONAL_LIGHT)
            {
                /*float colorScale = 6.f;
                PS_MATERIAL_BUFFER material;
                XMFLOAT4 col = XMFLOAT4(m_lightData.lights[i].color.x * colorScale, m_lightData.lights[i].color.y * colorScale, m_lightData.lights[i].color.z * colorScale, 1.f);
                material.ambient = col;
                material.diffuse = col;
                material.specular = col;
                material.emissive = col;
                m_renderObjects[SPOT_MESH].setMaterial(material);

                XMVECTOR position = XMVectorSet(0,0,0,1);
                XMVECTOR newPos = (-2.f * 50.f * XMLoadFloat4(&m_lightData.lights[i].direction)) + position;

                XMMATRIX worldMatrix = XMMatrixScaling(2.f, 2.f, 2.f);
                worldMatrix *= lookAtMatrix(newPos, XMVector3Normalize(XMVectorSetW(XMLoadFloat4(&m_lightData.lights[i].direction), 0.f)), XMVectorSet(0, 1, 0, 0));

                m_renderObjects[SPOT_MESH].updateWCPBuffer(worldMatrix, viewMatrix, projMatrix);
                m_renderObjects[SPOT_MESH].render(true);*/
            }
        }
    }
};

#endif // !LIGHTMANAGER_H