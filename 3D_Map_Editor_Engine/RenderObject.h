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

public:
	RenderObject();
	~RenderObject();

	// Initialization
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int id, std::string modelName);

	// Picking
	float pick(XMVECTOR rayOrigin, XMVECTOR rayDirection, char dimension = 'n');

	// Setters
	void setMaterial(PS_MATERIAL_BUFFER material);
	void setMaterialWithID(PS_MATERIAL_BUFFER material, int ID);
	void setTextures(TexturePaths textures);

	// Update
	void updateWCPBuffer(XMMATRIX worldMatrix, XMMATRIX viewProjMatrix);

	// Render
	void render(bool disableModelShaders = false);
};

#endif // !RENDEROBJECT_H