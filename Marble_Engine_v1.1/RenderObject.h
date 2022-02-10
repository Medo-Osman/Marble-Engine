#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include "Shaders.h"
#include "Model.h"

class RenderObject
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// ID
	int m_id;

	// Model
	Model* m_model;

	// Buffers
	Buffer<VS_WVP_CBUFFER> m_wvpCBuffer;

	// Shaders
	Shaders m_shaders;

	// Enabled
	bool m_enabled = true;

public:
	RenderObject();
	~RenderObject();

	// Initialization
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int id, std::string modelName, std::vector<MeshData>* meshData = nullptr);

	// Picking
	float pick(XMVECTOR rayOrigin, XMVECTOR rayDirection, char dimension = 'n');

	// Getters
	void materialUIUpdate();

	// Setters
	void setShaderState(ShaderStates shaderState);
	void setMaterial(PS_MATERIAL_BUFFER material);
	void setMaterial(PS_MATERIAL_PBR_BUFFER material);
	void setMaterialWithID(PS_MATERIAL_BUFFER material, int ID);
	void setMaterialWithID(PS_MATERIAL_PBR_BUFFER material, int ID);
	void setTextures(TexturePaths textures);
	void setTextures(TexturePathsPBR textures);
	void setEnabled(bool enabled);

	// Update
	void updateWCPBuffer(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX ProjMatrix);
	void fillMeshData(std::vector<MeshData>* meshes);

	// Render
	void render(bool disableModelShaders = false);
};

#endif // !RENDEROBJECT_H