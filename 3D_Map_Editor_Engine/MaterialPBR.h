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

enum class PBRTexturesTypes { ALBEDO, NORMAL, METALLIC, ROUGHNESS, EMISSIVE, AMBIENT_OCCLUSION, DISPLACEMENT, NONE };

class MaterialPBR
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Name
	std::string m_name;

	// Texture Picker
	ImGui::FileBrowser m_fileDialog;

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
			m_albedoTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.albedoPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_albedoTexture = ResourceHandler::getInstance().getTexture(L"DefaultWhite.bmp");
			m_albedoTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultWhite.bmp");
		}

		if (texturePaths.normalPath != L"")
		{
			m_normalTexture = ResourceHandler::getInstance().getTexture(texturePaths.normalPath.c_str());
			m_normalTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.normalPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_normalTexture = ResourceHandler::getInstance().getTexture(L"DefaultBlack.bmp");
			m_normalTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultBlack.bmp");
		}

		if (texturePaths.metallicPath != L"")
		{
			m_metallicTexture = ResourceHandler::getInstance().getTexture(texturePaths.metallicPath.c_str());
			m_metallicTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.metallicPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_metallicTexture = ResourceHandler::getInstance().getTexture(L"DefaultGrey.bmp");
			m_metallicTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultGrey.bmp");
		}

		if (texturePaths.roughnessPath != L"")
		{
			m_roughnessTexture = ResourceHandler::getInstance().getTexture(texturePaths.roughnessPath.c_str());
			m_roughnessTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.roughnessPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_roughnessTexture = ResourceHandler::getInstance().getTexture(L"DefaultGrey.bmp");
			m_roughnessTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultGrey.bmp");
		}

		if (texturePaths.emissivePath != L"")
		{
			m_emissiveTexture = ResourceHandler::getInstance().getTexture(texturePaths.emissivePath.c_str());
			m_emissiveTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.emissivePath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_emissiveTexture = ResourceHandler::getInstance().getTexture(L"DefaultBlack.bmp");
			m_emissiveTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultBlack.bmp");
		}

		if (texturePaths.ambientOcclusionPath != L"")
		{
			m_ambientOcclusionTexture = ResourceHandler::getInstance().getTexture(texturePaths.ambientOcclusionPath.c_str());
			m_ambientOcclusionTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.ambientOcclusionPath.c_str());
			m_materialData.materialTextured = true;
		}
		else
		{
			m_ambientOcclusionTexture = ResourceHandler::getInstance().getTexture(L"DefaultBlack.bmp");
			m_ambientOcclusionTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultBlack.bmp");
		}

		if (texturePaths.displacementPath != L"")
		{
			m_displacementTexture = ResourceHandler::getInstance().getTexture(texturePaths.displacementPath.c_str());
			m_displacementTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.displacementPath.c_str());
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

		m_fileDialog.SetTitle("Load Texture");
		m_fileDialog.SetTypeFilters({ ".png", ".jpg", ".jpeg", ".tga", ".dds", ".DDS" });
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

	void updateUI()
	{
		float imageSize = 50;
		float imageOffset = imageSize + 10;
		UINT nameSize = 64;
		WCHAR name[64];
		PBRTexturesTypes texType = PBRTexturesTypes::NONE;

		if (ImGui::CollapsingHeader("Textures"))
		{
			ImGui::Image(m_albedoTexture, ImVec2(imageSize, imageSize));
			m_albedoTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			_bstr_t nameCStr(name);
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::ALBEDO;
			}

			ImGui::Image(m_normalTexture, ImVec2(imageSize, imageSize));
			m_normalTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			nameCStr = name;
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::NORMAL;
			}

			ImGui::Image(m_metallicTexture, ImVec2(imageSize, imageSize));
			m_metallicTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			nameCStr = name;
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::METALLIC;
			}

			ImGui::Image(m_roughnessTexture, ImVec2(imageSize, imageSize));
			m_roughnessTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			nameCStr = name;
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::ROUGHNESS;
			}

			ImGui::Image(m_emissiveTexture, ImVec2(imageSize, imageSize));
			m_emissiveTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			nameCStr = name;
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::EMISSIVE;
			}

			ImGui::Image(m_ambientOcclusionTexture, ImVec2(imageSize, imageSize));
			m_ambientOcclusionTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			nameCStr = name;
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::AMBIENT_OCCLUSION;
			}

			ImGui::Image(m_displacementTexture, ImVec2(imageSize, imageSize));
			m_displacementTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
			nameCStr = name;
			ImGui::SameLine(imageOffset);
			if (ImGui::Button((const char*)nameCStr))
			{
				m_fileDialog.Open();
				m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
				texType = PBRTexturesTypes::DISPLACEMENT;
			}

		}
		m_fileDialog.Display();

		if (m_fileDialog.HasSelected())
		{
			std::wstring path = charToWchar(m_fileDialog.GetSelected().string());
			size_t pos = path.find(L"Textures");
			path.erase(0, pos);
			switch (texType)
			{
			case PBRTexturesTypes::ALBEDO:
				m_albedoTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_albedoTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			case PBRTexturesTypes::NORMAL:
				m_normalTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_normalTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			case PBRTexturesTypes::METALLIC:
				m_metallicTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_metallicTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			case PBRTexturesTypes::ROUGHNESS:
				m_roughnessTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_roughnessTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			case PBRTexturesTypes::EMISSIVE:
				m_emissiveTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_emissiveTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			case PBRTexturesTypes::AMBIENT_OCCLUSION:
				m_ambientOcclusionTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_ambientOcclusionTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			case PBRTexturesTypes::DISPLACEMENT:
				m_displacementTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_displacementTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				break;
			default:
				break;
			}

			m_fileDialog.ClearSelected();
		}
	}
};

#endif // !MATERIAL_PBR_H