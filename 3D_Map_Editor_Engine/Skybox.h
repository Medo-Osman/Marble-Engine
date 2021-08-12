#include "pch.h"
#ifndef SKYBOX_H
#define SKYBOX_H

#include "RenderObject.h"

class Skybox
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Texture
	ComPtr< ID3D11Texture2D > m_skymapTexture;
	ComPtr< ID3D11ShaderResourceView > m_textureShaderResourceView;

	// Mesh
	Mesh<VertexPos>* m_cubeMesh;

	// Buffers
	Buffer<VS_SKYBOX_MATRIX_CBUFFER> m_vpCBuffer; // Includes rotation

	// Shaders
	Shaders m_shaders;

	// Skybox Rotation
	XMVECTOR m_rotation;

public:
	Skybox()
	{
		m_deviceContext = nullptr;
		m_cubeMesh = nullptr;
		m_rotation = XMVectorZero();
	}
	~Skybox()
	{
		delete m_cubeMesh;
	}

	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::wstring texturePath)
	{
		// Device
		m_deviceContext = deviceContext;

		// Shaders
		ShaderFiles shaders;
		shaders.vs = L"SkyboxVS.hlsl";
		shaders.ps = L"SkyboxPS.hlsl";
		m_shaders.initialize(device, m_deviceContext, shaders, LayoutType::POS);

		// Cube
		std::vector<VertexPos> vertices =
		{
			// Back
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f) },

			// Front
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },

			// Left
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f) }, 
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },

			// Right
			{ XMFLOAT3(1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },

			// Bottom
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },

			// Top
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f) }
		};

		// Mesh
		PS_MATERIAL_BUFFER material;
		std::vector<UINT> indices;
		m_cubeMesh = new Mesh<VertexPos>(device, m_deviceContext, vertices, indices, material, TexturePaths());

		// Constant Buffer
		m_vpCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);
		
		// Texture
		HRESULT hr = CreateDDSTextureFromFile(device, texturePath.c_str(), (ID3D11Resource**)m_skymapTexture.GetAddressOf(), &m_textureShaderResourceView);
		assert(SUCCEEDED(hr) && "Error, skymap texture failed to load!");

		D3D11_TEXTURE2D_DESC skymapTextureDesc;
		m_skymapTexture->GetDesc(&skymapTextureDesc);
		D3D11_SHADER_RESOURCE_VIEW_DESC textureShaderResourceViewDesc;
		textureShaderResourceViewDesc.Format = skymapTextureDesc.Format;
		textureShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		textureShaderResourceViewDesc.TextureCube.MipLevels = skymapTextureDesc.MipLevels;
		textureShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;

		hr = device->CreateShaderResourceView(m_skymapTexture.Get(), &textureShaderResourceViewDesc, &m_textureShaderResourceView);
		assert(SUCCEEDED(hr) && "Error, failed to create skymap shader resource view!");

		// Constant Buffers
		m_vpCBuffer.initialize(device, m_deviceContext, nullptr, BufferType::CONSTANT);

		// Rotation
		m_rotation = XMVectorSet(0.f, XMConvertToRadians(160.f), 0.f, 0.f);
	}

	void updateVP(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
	{
		VS_SKYBOX_MATRIX_CBUFFER* vpData = new VS_SKYBOX_MATRIX_CBUFFER();

		XMMATRIX worldMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation); // Rotation only
		vpData->vpMatrix = worldMatrix * viewMatrix * projectionMatrix;
		m_vpCBuffer.update(&vpData);
	}

	void render()
	{
		// Setting Shaders
		this->m_shaders.setShaders();

		// Constant Buffer
		this->m_deviceContext->VSSetConstantBuffers(0, 1, m_vpCBuffer.GetAddressOf());

		// Set Texture
		this->m_deviceContext->PSSetShaderResources(0, 1, m_textureShaderResourceView.GetAddressOf());

		// Render
		m_cubeMesh->render();
	}
};

#endif // !SKYBOX_H
