#include "pch.h"
#ifndef SKYBOX_H
#define SKYBOX_H

#include "RenderObject.h"

enum class CubemapType { Skybox, Irradiance, None };

class Skybox
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Texture
	// - Skybox
	std::wstring m_skyboxTexturePath;
	ComPtr< ID3D11ShaderResourceView > m_skyboxTextureSRV;
	// - Irradiance
	std::wstring m_irradianceTexturePath;
	ComPtr< ID3D11ShaderResourceView > m_irradianceTextureSRV;
	// - Previews
	RenderTexture m_skyboxPreviewTexture;
	RenderTexture m_irradiancePreviewTexture;
	D3D11_VIEWPORT m_previewViewport;
	UINT m_previewTextureWidth = 100;
	UINT m_previewTextureHeight = 100;

	// Mesh
	Mesh<VertexPos>* m_cubeMesh;

	// Buffers
	Buffer<VS_SKYBOX_MATRIX_CBUFFER> m_vpCBuffer; // Includes rotation

	// Shaders
	Shaders m_shaders;
	Shaders m_previewShaders;

	// Skybox Rotation
	XMVECTOR m_rotation;

	// Functions
	void initPreviewTexture(ID3D11Device* device, RenderTexture& rtv)
	{
		// Texture
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = m_previewTextureWidth;
		textureDesc.Height = m_previewTextureHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = m_skyboxPreviewTexture.format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		HRESULT hr = device->CreateTexture2D(&textureDesc, NULL, &rtv.rtt);
		assert(SUCCEEDED(hr) && "Error, render target texture could not be created!");

		// Render Rarget View
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		hr = device->CreateRenderTargetView(rtv.rtt, &renderTargetViewDesc, &rtv.rtv);
		assert(SUCCEEDED(hr) && "Error, render target view could not be created!");

		// Shader Resource View
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(rtv.rtt, &srvDesc, &rtv.srv);
		assert(SUCCEEDED(hr) && "Error, shader resource view could not be created!");
	}

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

	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::wstring skyboxTexturePath, std::wstring irradianceTexturePath)
	{
		// Device
		m_deviceContext = deviceContext;

		// Shaders
		ShaderFiles shaders;
		shaders.vs = L"SkyboxVS.hlsl";
		shaders.ps = L"SkyboxPS.hlsl";
		m_shaders.initialize(device, m_deviceContext, shaders, LayoutType::POS);

		shaders.vs = L"FullscreenQuadVS.hlsl";
		shaders.ps = L"SkyboxPreviewPS.hlsl";
		m_previewShaders.initialize(device, m_deviceContext, shaders, LayoutType::POS);

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
		// - Skybox Texture
		m_skyboxTexturePath = skyboxTexturePath;
		m_skyboxTextureSRV = ResourceHandler::getInstance().getTexture(skyboxTexturePath.c_str());
		initPreviewTexture(device, m_skyboxPreviewTexture);
		/*HRESULT hr = CreateDDSTextureFromFile(device, skyboxTexturePath.c_str(), (ID3D11Resource**)m_skyboxTexture.GetAddressOf(), &m_skyboxTextureShaderResourceView);
		assert(SUCCEEDED(hr) && "Error, skybox texture failed to load!");
		D3D11_TEXTURE2D_DESC skymapTextureDesc;
		m_skyboxTexture->GetDesc(&skymapTextureDesc);
		D3D11_SHADER_RESOURCE_VIEW_DESC textureShaderResourceViewDesc;
		textureShaderResourceViewDesc.Format = skymapTextureDesc.Format;
		textureShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		textureShaderResourceViewDesc.TextureCube.MipLevels = skymapTextureDesc.MipLevels;
		textureShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		hr = device->CreateShaderResourceView(m_skyboxTexture.Get(), &textureShaderResourceViewDesc, &m_skyboxTextureShaderResourceView);
		assert(SUCCEEDED(hr) && "Error, failed to create skybox shader resource view!");*/

		// - Irradiance Texture
		m_irradianceTexturePath = irradianceTexturePath;
		m_irradianceTextureSRV = ResourceHandler::getInstance().getTexture(irradianceTexturePath.c_str());
		initPreviewTexture(device, m_irradiancePreviewTexture);
		/*hr = CreateDDSTextureFromFile(device, irradianceTexturePath.c_str(), (ID3D11Resource**)m_irradianceTexture.GetAddressOf(), &m_irradianceTextureShaderResourceView);
		assert(SUCCEEDED(hr) && "Error, irradiance skybox texture failed to load!");
		m_skyboxTexture->GetDesc(&skymapTextureDesc);
		textureShaderResourceViewDesc.Format = skymapTextureDesc.Format;
		textureShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		textureShaderResourceViewDesc.TextureCube.MipLevels = skymapTextureDesc.MipLevels;
		textureShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		hr = device->CreateShaderResourceView(m_skyboxTexture.Get(), &textureShaderResourceViewDesc, &m_skyboxTextureShaderResourceView);
		assert(SUCCEEDED(hr) && "Error, failed to create irradiance skybox shader resource view!");*/
		
		// - Preivew Viewport
		m_previewViewport.TopLeftX = 0.f;
		m_previewViewport.TopLeftY = 0.f;
		m_previewViewport.Width = (FLOAT)m_previewTextureWidth;
		m_previewViewport.Height = (FLOAT)m_previewTextureHeight;
		m_previewViewport.MinDepth = 0.f;
		m_previewViewport.MaxDepth = 1.f;

		// Constant Buffers
		m_vpCBuffer.initialize(device, m_deviceContext, nullptr, BufferType::CONSTANT);

		// Rotation
		//m_rotation = XMVectorSet(0.f, XMConvertToRadians(160.f), 0.f, 0.f);
	}

	std::wstring getSkyboxFileName() const { return m_skyboxTexturePath; }
	std::wstring getIrradianceFileName() const { return m_irradianceTexturePath; }
	ID3D11ShaderResourceView* getSkyboxPreviewSRV() const { return m_skyboxPreviewTexture.srv; }
	ID3D11ShaderResourceView* getIrradiancePreviewSRV() const { return m_irradiancePreviewTexture.srv; }

	void updateSkyboxCubemap(std::wstring path)
	{
		m_skyboxTextureSRV = ResourceHandler::getInstance().getTexture(path.c_str());
		m_skyboxTexturePath = path;
	}
	void updateIrradianceCubemap(std::wstring path)
	{
		m_irradianceTextureSRV = ResourceHandler::getInstance().getTexture(path.c_str());
		m_irradianceTexturePath = path;
	}
	void updateVP(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
	{
		VS_SKYBOX_MATRIX_CBUFFER* vpData = new VS_SKYBOX_MATRIX_CBUFFER();

		XMMATRIX worldMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation); // Rotation only
		vpData->vpMatrix = XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);
		m_vpCBuffer.update(&vpData);
	}
	void updatePreviewShaders()
	{
		m_previewShaders.updateShaders();
	}

	void setSkyboxTextures(int skyboxRegister, int irradianceRegister)
	{
		m_deviceContext->PSSetShaderResources(skyboxRegister, 1, m_skyboxTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(irradianceRegister, 1, m_irradianceTextureSRV.GetAddressOf());
	}
	void cubemapPreviewsRenderSetup()
	{
		// Set Viewport
		m_deviceContext->RSSetViewports(1, &m_previewViewport);
		
		// Setting Shaders
		m_previewShaders.setShaders();

		// Set RenderTargets
		ID3D11RenderTargetView* renderTargets[] = {
			m_skyboxPreviewTexture.rtv,
			m_irradiancePreviewTexture.rtv
		};
		m_deviceContext->OMSetRenderTargets(2, renderTargets, nullptr);

		// Set Textures
		m_deviceContext->PSSetShaderResources(0, 1, m_skyboxTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(1, 1, m_irradianceTextureSRV.GetAddressOf());
	}
	void render()
	{
		// Setting Shaders
		m_shaders.setShaders();

		// Constant Buffer
		m_deviceContext->VSSetConstantBuffers(0, 1, m_vpCBuffer.GetAddressOf());

		// Set Texture
		//m_deviceContext->PSSetShaderResources(0, 1, m_skyboxTextureSRV.GetAddressOf());
		// Already set to slot 6 as SpecularIBLMap

		// Render
		m_cubeMesh->render();
	}
};

#endif // !SKYBOX_H
