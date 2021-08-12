#include "pch.h"
#ifndef MODELSELECTIONHANDLER_H
#define MODELSELECTIONHANDLER_H

#include "RenderObject.h"

class ModelSelectionHandler
{
private:
    // Device
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    
    // Render Objects

    // - Arrows
    RenderObject m_arrowModelX;
    RenderObject m_arrowModelY;
    RenderObject m_arrowModelZ;

    XMMATRIX m_arrowMatrixX;
    XMMATRIX m_arrowMatrixY;
    XMMATRIX m_arrowMatrixZ;

    bool m_objectSelected;
    XMFLOAT3 m_selectedModelPos;

    // Camera Matrices
    XMMATRIX* m_viewMatrix;
    XMMATRIX* m_projectionMatrix;

public:
    ModelSelectionHandler()
    {
        m_device = nullptr;
        m_deviceContext = nullptr;

        m_arrowMatrixX = XMMatrixIdentity();
        m_arrowMatrixY = XMMatrixIdentity();
        m_arrowMatrixZ = XMMatrixIdentity();

        m_objectSelected = false;
        m_selectedModelPos = { 0.f, 0.f, 0.f };

        m_viewMatrix = nullptr;
        m_projectionMatrix = nullptr;
    }
    ~ModelSelectionHandler() = default;

    // Initialization
    void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX* viewMatrix, XMMATRIX* projectionMatrix)
    {
        m_device = device;
        m_deviceContext = deviceContext;

        m_viewMatrix = viewMatrix;
        m_projectionMatrix = projectionMatrix;

        PS_MATERIAL_BUFFER mat;

        // X Red
        m_arrowModelX.initialize(m_device, m_deviceContext, 9999, "arrow.obj");
        mat.ambient = { 1.f, 0.f, 0.f, .6f };
        mat.diffuse = { 1.f, 0.f, 0.f, .6f };
        mat.specular = { 0.f, 0.f, 0.f, 0.f };
        m_arrowModelX.setMaterialWithID(mat, 0);
        mat.ambient = { 0.8f, 0.f, 0.f, .6f };
        mat.diffuse = { 0.8f, 0.f, 0.f, .6f };
        m_arrowModelX.setMaterialWithID(mat, 1);

        // Y Blue
        m_arrowModelY.initialize(m_device, m_deviceContext, 9998, "arrow.obj");
        mat.ambient = { 0.f, 0.f, 1.f, .6f };
        mat.diffuse = { 0.f, 0.f, 1.f, .6f };
        mat.specular = { 0.f, 0.f, 0.f, 0.f };
        m_arrowModelY.setMaterialWithID(mat, 0);
        mat.ambient = { 0.f, 0.f, 0.8f, .6f };
        mat.diffuse = { 0.f, 0.f, 0.8f, .6f };
        m_arrowModelY.setMaterialWithID(mat, 1);
        
        // Z Green
        m_arrowModelZ.initialize(m_device, m_deviceContext, 9997, "arrow.obj");
        mat.ambient = { 0.f, 1.f, 0.f, .6f };
        mat.diffuse = { 0.f, 1.f, 0.f, .6f };
        mat.specular = { 0.f, 0.f, 0.f, 0.f };
        m_arrowModelZ.setMaterialWithID(mat, 0);
        mat.ambient = { 0.f, 0.8f, 0.f, .6f };
        mat.diffuse = { 0.f, 0.8f, 0.f, .6f };
        m_arrowModelZ.setMaterialWithID(mat, 1);
    }

    // Picking
    float picking(XMVECTOR rayOrigin, XMVECTOR rayDirection, char dimension)
    {
        switch (dimension)
        {
        case 'x':
            return m_arrowModelX.pick(rayOrigin, rayDirection, dimension);
            break;

        case 'y':
            return m_arrowModelY.pick(rayOrigin, rayDirection, dimension);
            break;

        case 'z':
            return m_arrowModelZ.pick(rayOrigin, rayDirection, dimension);
            break;

        default:
            assert(!"Error, invalid selection arrow dimension!");
            break;
        }
    }

    // Getters
    XMMATRIX getWorldMatrix(char dimension)
    {
        switch (dimension)
        {
        case 'x':
            return m_arrowMatrixX;
            break;
        case 'y':
            return m_arrowMatrixY;
            break;
        case 'z':
            return m_arrowMatrixZ;
            break;
        default:
            assert(!"Error, invalid selection arrow dimension!");
            break;
        }
    }

    // Selection
    void objectSelected(XMFLOAT3 position)
    {
        m_objectSelected = true;
        m_selectedModelPos = position;
    }
    void objectDeselected() { m_objectSelected = false; }

    // Render
    void renderArrows()
    {
        if (m_objectSelected)
        {
            XMMATRIX viewProjMatrix = *m_viewMatrix * *m_projectionMatrix;
            m_arrowMatrixX = XMMatrixScaling(0.5f, 0.5f, 0.5f) *
                XMMatrixRotationRollPitchYaw(XM_PIDIV2, XM_PIDIV2, 0.f) *
                XMMatrixTranslation(m_selectedModelPos.x + 1.f, m_selectedModelPos.y, m_selectedModelPos.z);
            m_arrowModelX.updateWCPBuffer(m_arrowMatrixX, viewProjMatrix);
        
            m_arrowMatrixY = XMMatrixScaling(0.5f, 0.5f, 0.5f) *
                XMMatrixTranslation(m_selectedModelPos.x, m_selectedModelPos.y + 1.f, m_selectedModelPos.z);
            m_arrowModelY.updateWCPBuffer(m_arrowMatrixY, viewProjMatrix);

            m_arrowMatrixZ = XMMatrixScaling(0.5f, 0.5f, 0.5f) *
                XMMatrixRotationRollPitchYaw(XM_PIDIV2, 0.f, 0.f) *
                XMMatrixTranslation(m_selectedModelPos.x, m_selectedModelPos.y, m_selectedModelPos.z + 1.f);
            m_arrowModelZ.updateWCPBuffer(m_arrowMatrixZ, viewProjMatrix);

            m_arrowModelX.render();
            m_arrowModelY.render();
            m_arrowModelZ.render();
        }
    }
};

#endif // !MODELSELECTIONHANDLER_H