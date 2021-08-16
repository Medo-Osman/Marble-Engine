#include "pch.h"
#ifndef MATERIAL_PBR_H
#define MATERIAL_PBR_H

#include "ResourceHandler.h"
#include "Buffer.h"

struct TexturePathsPBR
{
	std::wstring albedoPath = L"";
	std::wstring normalPath = L"";
	std::wstring metallicPath = L"";
	std::wstring roughnessPath = L"";
	std::wstring emissivePath = L"";
	std::wstring ambientOcclusionPath = L"";
	std::wstring displacementPath = L"";
};

struct PS_MATERIAL_PBR_BUFFER
{
	DirectX::XMFLOAT3 albedo = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	float metallic = 0.f;
	float roughness = 0.f;
	float emissiveStrength = 0.f;
	BOOL materialTextured = true;
	int pad = 0;
};

class MaterialPBR
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Name
	std::string m_name;

	// Testures
	ID3D11ShaderResourceView* m_albedoTexture;
	ID3D11ShaderResourceView* m_normalTexture;
	ID3D11ShaderResourceView* m_metallicTexture;
	ID3D11ShaderResourceView* m_roughnessTexture;
	ID3D11ShaderResourceView* m_emissiveTexture;
	ID3D11ShaderResourceView* m_ambientOcclusionTexture;
	ID3D11ShaderResourceView* m_displacementTexture;
	bool m_displacementExists;

	// Constant Buffer
	PS_MATERIAL_PBR_BUFFER m_materialData;
	Buffer<PS_MATERIAL_PBR_BUFFER> m_materialCBuffer;

	// Helper Functions
	void loadTextures(TexturePathsPBR texturePaths)
	{
		if (texturePaths.albedoPath != L"")
		{
			m_albedoTexture = ResourceHandler::getInstance().getTexture(texturePaths.albedoPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_albedoTexture = ResourceHandler::getInstance().getTexture(L"DefaultWhite.bmp");
		}

		if (texturePaths.normalPath != L"")
		{
			m_normalTexture = ResourceHandler::getInstance().getTexture(texturePaths.normalPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_normalTexture = ResourceHandler::getInstance().getTexture(L"DefaultBlack.bmp");
		}

		if (texturePaths.metallicPath != L"")
		{
			m_metallicTexture = ResourceHandler::getInstance().getTexture(texturePaths.metallicPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_metallicTexture = ResourceHandler::getInstance().getTexture(L"DefaultGrey.bmp");
		}

		if (texturePaths.roughnessPath != L"")
		{
			m_roughnessTexture = ResourceHandler::getInstance().getTexture(texturePaths.roughnessPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_roughnessTexture = ResourceHandler::getInstance().getTexture(L"DefaultGrey.bmp");
		}

		if (texturePaths.emissivePath != L"")
		{
			m_emissiveTexture = ResourceHandler::getInstance().getTexture(texturePaths.emissivePath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_emissiveTexture = ResourceHandler::getInstance().getTexture(L"DefaultBlack.bmp");
		}

		if (texturePaths.ambientOcclusionPath != L"")
		{
			m_ambientOcclusionTexture = ResourceHandler::getInstance().getTexture(texturePaths.ambientOcclusionPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_ambientOcclusionTexture = ResourceHandler::getInstance().getTexture(L"DefaultBlack.bmp");
		}

		if (texturePaths.displacementPath != L"")
		{
			m_displacementTexture = ResourceHandler::getInstance().getTexture(texturePaths.displacementPath.c_str());
		}
	}

public:
	MaterialPBR()
	{
		m_deviceContext				= nullptr;
		m_normalTexture				= nullptr;
		m_metallicTexture			= nullptr;
		m_roughnessTexture			= nullptr;
		m_emissiveTexture			= nullptr;
		m_ambientOcclusionTexture	= nullptr;
		m_displacementTexture		= nullptr;
		m_displacementExists		= false;
	}

	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, PS_MATERIAL_PBR_BUFFER material, TexturePathsPBR texturePaths)
	{
		// Device
		m_deviceContext = deviceContext;

		// Material data
		m_materialData = material;

		// Textures
		loadTextures(texturePaths);

		// Constant Buffer
		m_materialCBuffer.initialize(device, m_deviceContext, &m_materialData, BufferType::CONSTANT);
	}

	std::string getName()
	{
		return m_name;
	}

	void setMaterial(PS_MATERIAL_PBR_BUFFER material)
	{
		m_materialData = material;
		
		PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
		m_materialCBuffer.update(&materialData);
	}
	void setTextures(TexturePathsPBR texturePaths)
	{
		loadTextures(texturePaths);
		PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
		m_materialCBuffer.update(&materialData);
	}

	void sendCBufferAndTextures()
	{
		// Pixel Shader: Slot 0, Light buffer is set to Slot 2, Camera Position buffer is set to Slot 1
		m_deviceContext->PSSetConstantBuffers(0, 1, m_materialCBuffer.GetAddressOf());

		// Testures
		if (m_materialData.materialTextured)
		{
			m_deviceContext->PSSetShaderResources(0, 1, &m_albedoTexture);
			m_deviceContext->PSSetShaderResources(1, 1, &m_normalTexture);
			m_deviceContext->PSSetShaderResources(2, 1, &m_metallicTexture);
			m_deviceContext->PSSetShaderResources(3, 1, &m_roughnessTexture);
			m_deviceContext->PSSetShaderResources(4, 1, &m_emissiveTexture);
			m_deviceContext->PSSetShaderResources(5, 1, &m_ambientOcclusionTexture);
		}

		if (m_displacementExists)
			m_deviceContext->DSSetShaderResources(0, 1, &m_displacementTexture);
	}
	void setName(std::string newName)
	{
		m_name = newName;
	}
};

#endif // !MATERIAL_PBR_H