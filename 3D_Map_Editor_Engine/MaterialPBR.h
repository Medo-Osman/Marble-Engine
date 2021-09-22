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

/*
struct PS_MATERIAL_PBR_BUFFER
{
	DirectX::XMFLOAT3 albedo = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	float metallic = 0.f;
	float roughness = 0.f;
	float emissiveStrength = 0.f;
	BOOL albedoTexture = true;
	BOOL normalTexture = true;
	BOOL metallicTexture = true;
	BOOL roughnessTexture = true;
	XMFLOAT2 pad = XMFLOAT2();
};

*/

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
	PBRTexturesTypes m_texTypeToLoad;

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
			m_normalTexture = ResourceHandler::getInstance().getTexture(L"DefaultNormal.dds");
			m_normalTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultNormal.dds");
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
			m_ambientOcclusionTexture = ResourceHandler::getInstance().getTexture(L"DefaultWhite.bmp");
			m_ambientOcclusionTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, L"DefaultWhite.bmp");
		}

		if (texturePaths.displacementPath != L"")
		{
			m_displacementTexture = ResourceHandler::getInstance().getTexture(texturePaths.displacementPath.c_str());
			m_displacementTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.displacementPath.c_str());
		}
	}
	void UIUseTextureCheckbox(BOOL& textureExistsBool)
	{
		if (ImGui::Checkbox(" ", (bool*)&textureExistsBool))
		{
			PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
			m_materialCBuffer.update(&materialData);
		}
	}

public:
	MaterialPBR()
	{
		m_deviceContext				= nullptr;
		m_albedoTexture				= nullptr;
		m_normalTexture				= nullptr;
		m_metallicTexture			= nullptr;
		m_roughnessTexture			= nullptr;
		m_emissiveTexture			= nullptr;
		m_ambientOcclusionTexture	= nullptr;
		m_displacementTexture		= nullptr;
		m_displacementExists		= false;

		m_fileDialog.SetTitle("Load Texture");
		m_fileDialog.SetTypeFilters({ ".png", ".jpg", ".jpeg", ".tga", ".dds", ".DDS", ".bmp" });
		m_texTypeToLoad = PBRTexturesTypes::NONE;
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
		float offset = 60.f;
		float imageOffset = ImGui::GetWindowSize().x - ImGui::GetContentRegionMax().x + imageSize + offset;
		UINT nameSize = 64;
		WCHAR name[64];
		PBRTexturesTypes texType = PBRTexturesTypes::NONE;

		if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			_bstr_t nameCStr(name);
			// Albedo
			if (ImGui::TreeNodeEx((void*)"Albedo", ImGuiTreeNodeFlags_DefaultOpen, "Albedo"))
			{
				if (m_albedoTexture)
				{
					ImGui::Image(m_albedoTexture, ImVec2(imageSize, imageSize));
					m_albedoTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				ImGui::BeginGroup();
				UIUseTextureCheckbox(m_materialData.materialTextured);
				ImGui::SameLine(30);
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::ALBEDO;
				}
				if (ImGui::ColorEdit3("Color##2f", &m_materialData.albedo.x, ImGuiColorEditFlags_Float))
				{
					PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}
				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Normal
			if (ImGui::TreeNodeEx((void*)"Normal", ImGuiTreeNodeFlags_DefaultOpen, "Normal"))
			{
				if (m_normalTexture)
				{
					ImGui::Image(m_normalTexture, ImVec2(imageSize, imageSize));
					m_normalTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::NORMAL;
				}
				ImGui::TreePop();
			}

			// Metallic
			if (ImGui::TreeNodeEx((void*)"Metallic", ImGuiTreeNodeFlags_DefaultOpen, "Metallic"))
			{
				if (m_metallicTexture)
				{
					ImGui::Image(m_metallicTexture, ImVec2(imageSize, imageSize));
					m_metallicTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				ImGui::BeginGroup();
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::METALLIC;
				}
				if (ImGui::DragFloat("Value", &m_materialData.metallic, 0.01f))
				{
					PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}
				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Rougness
			if (ImGui::TreeNodeEx((void*)"Rougness", ImGuiTreeNodeFlags_DefaultOpen, "Rougness"))
			{
				if (m_roughnessTexture)
				{
					ImGui::Image(m_roughnessTexture, ImVec2(imageSize, imageSize));
					m_roughnessTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				ImGui::BeginGroup();
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::ROUGHNESS;
				}
				if (ImGui::DragFloat("Value", &m_materialData.roughness, 0.01f))
				{
					PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}
				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Emissive
			if (ImGui::TreeNodeEx((void*)"Emissive", ImGuiTreeNodeFlags_DefaultOpen, "Emissive"))
			{
				if (m_emissiveTexture)
				{
					ImGui::Image(m_emissiveTexture, ImVec2(imageSize, imageSize));
					m_emissiveTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				ImGui::BeginGroup();
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::EMISSIVE;
				}
				if (ImGui::DragFloat("Strength", &m_materialData.emissiveStrength, 0.1f))
				{
					PS_MATERIAL_PBR_BUFFER* materialData = new PS_MATERIAL_PBR_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}
				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Ambient Occlusion
			if (ImGui::TreeNodeEx((void*)"Ambient Occlusion", ImGuiTreeNodeFlags_DefaultOpen, "Ambient Occlusion"))
			{
				if (m_ambientOcclusionTexture)
				{
					ImGui::Image(m_ambientOcclusionTexture, ImVec2(imageSize, imageSize));
					m_ambientOcclusionTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::AMBIENT_OCCLUSION;
				}
				ImGui::TreePop();
			}

			// Displacement
			if (ImGui::TreeNodeEx((void*)"Displacement", ImGuiTreeNodeFlags_DefaultOpen, "Displacement"))
			{
				if (m_displacementTexture)
				{
					ImGui::Image(m_displacementTexture, ImVec2(imageSize, imageSize));
					m_displacementTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
					nameCStr = name;
				}
				else
				{
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"Empty_Texture.jpg"), ImVec2(imageSize, imageSize));
					nameCStr = "Add Texture";
				}
				ImGui::SameLine(imageOffset);
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PBRTexturesTypes::DISPLACEMENT;
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		m_fileDialog.Display();

		if (m_fileDialog.HasSelected())
		{
			std::string strPath = m_fileDialog.GetSelected().string();
			size_t pos = strPath.find("Textures");
			strPath.erase(0, pos);
			std::wstring path = charToWchar(strPath);

			switch (m_texTypeToLoad)
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