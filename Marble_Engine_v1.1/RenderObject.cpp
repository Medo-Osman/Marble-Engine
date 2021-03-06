#include "pch.h"
#include "RenderObject.h"

RenderObject::RenderObject()
{
	m_deviceContext = nullptr;
	m_model = nullptr;
	m_id = 0;
}

RenderObject::~RenderObject() {}

void RenderObject::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int id, std::string modelName, std::vector<MeshData>* meshData)
{
	// Device
	m_deviceContext = deviceContext;

	// ID
	m_id = id;

	// Shaders
	ShaderFiles shaders;
	shaders.vs = L"GeneralVS.hlsl";
	shaders.ps = L"GeneralPS.hlsl";
	m_shaders.initialize(device, m_deviceContext, shaders, LayoutType::POS_NOR_TEX_TAN);

	// Model
	m_model = new Model();
	m_model->initialize(device, deviceContext, m_id, modelName, meshData);

	// Constant Buffer
	m_wvpCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);
}

float RenderObject::pick(XMVECTOR rayOrigin, XMVECTOR rayDirection, char dimension)
{
	return m_model->pick(rayOrigin, rayDirection, dimension);
}

void RenderObject::materialUIUpdate()
{
	m_model->updateUI();
}

void RenderObject::setShaderState(ShaderStates shaderState)
{
	m_model->setShaderState(shaderState);
}

void RenderObject::setMaterial(PS_MATERIAL_BUFFER material)
{
	m_model->setMaterial(material);
}

void RenderObject::setMaterial(PS_MATERIAL_PBR_BUFFER material)
{
	m_model->setMaterial(material);
}

void RenderObject::setMaterialWithID(PS_MATERIAL_BUFFER material, int ID)
{
	m_model->setMaterialWithID(material, ID);
}

void RenderObject::setMaterialWithID(PS_MATERIAL_PBR_BUFFER material, int ID)
{
	m_model->setMaterialWithID(material, ID);
}

void RenderObject::setTextures(TexturePaths textures)
{
	m_model->setTexture(textures);
}

void RenderObject::setTextures(TexturePathsPBR textures)
{
	m_model->setTexture(textures);
}

void RenderObject::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

void RenderObject::updateWCPBuffer(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX ProjMatrix)
{
	VS_WVP_CBUFFER* wvpData = new VS_WVP_CBUFFER();
	wvpData->wvp = XMMatrixTranspose(worldMatrix * viewMatrix * ProjMatrix);
	wvpData->worldMatrix = XMMatrixTranspose(worldMatrix);

	wvpData->normalMatrix = DirectX::XMMatrixInverse(nullptr, worldMatrix);
	//wvpData->normalMatrix = XMMatrixTranspose(wvpData->normalMatrix/* * viewMatrix*/); // Normals wrong when mesh is rotated.

	m_wvpCBuffer.update(&wvpData);
}

void RenderObject::fillMeshData(std::vector<MeshData>* meshes)
{
	m_model->fillMeshData(meshes);
}

void RenderObject::render(bool disableModelShaders)
{
	if (m_enabled)
	{
		// Shaders
		if (!disableModelShaders)
			m_shaders.setShaders();
	
		// Constant Buffer
		m_deviceContext->VSSetConstantBuffers(0, 1, m_wvpCBuffer.GetAddressOf());

		// Model
		if (m_model)
			m_model->render();
	}
}