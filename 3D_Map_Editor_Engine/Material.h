#include "pch.h"
#ifndef MATERIAL_H
#define MATERIAL_H

#include "ResourceHandler.h"
#include "Buffer.h"

struct TexturePaths
{
	std::wstring diffusePath = L"";
	std::wstring specularPath = L"";
	std::wstring normalPath = L"";
	std::wstring displacementPath = L"";
};

struct PS_MATERIAL_BUFFER
{
	XMFLOAT4 emissive;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	float shininess;
	BOOL diffTextureExists;
	BOOL specTextureExists;
	BOOL normTextureExists;

	PS_MATERIAL_BUFFER()
	{
		emissive = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
		ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
		diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		specular = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		shininess = 30.f;
		diffTextureExists = false;
		specTextureExists = false;
		normTextureExists = false;
	}
};

enum class PhongTexturesTypes {DIFFUSE, SPECULAR, NORMAL, DISPLACEMENT, NONE};

class Material
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Name
	std::string m_name;

	// Texture Picker
	ImGui::FileBrowser m_fileDialog;
	PhongTexturesTypes m_texTypeToLoad;

	// Textures
	ID3D11ShaderResourceView* m_diffuseTexture;
	ID3D11ShaderResourceView* m_specularTexture;
	ID3D11ShaderResourceView* m_normalTexture;
	ID3D11ShaderResourceView* m_displacementTexture;

	// Constant Buffer
	PS_MATERIAL_BUFFER m_materialData;
	Buffer<PS_MATERIAL_BUFFER> m_materialCBuffer;
	XMFLOAT4 m_emissiveColor;

	// Helper Functions
	void loadTextures(TexturePaths texturePaths)
	{
		if (texturePaths.diffusePath != L"")
		{
			m_diffuseTexture = ResourceHandler::getInstance().getTexture(texturePaths.diffusePath.c_str());
			m_diffuseTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.diffusePath.c_str());
			m_materialData.diffTextureExists = true;
			m_diffTextureExists = true;
		}
		if (texturePaths.specularPath != L"")
		{
			m_specularTexture = ResourceHandler::getInstance().getTexture(texturePaths.specularPath.c_str());
			m_specularTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.specularPath.c_str());
			m_materialData.specTextureExists = true;
			m_specTextureExists = true;
		}
		if (texturePaths.normalPath != L"")
		{
			m_normalTexture = ResourceHandler::getInstance().getTexture(texturePaths.normalPath.c_str());
			m_normalTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.normalPath.c_str());
			m_materialData.normTextureExists = true;
			m_normTextureExists = true;
		}
		if (texturePaths.displacementPath != L"")
		{
			m_displacementTexture = ResourceHandler::getInstance().getTexture(texturePaths.displacementPath.c_str());
			m_displacementTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, texturePaths.displacementPath.c_str());
			m_displacementExists = true;
		}
	}
	void UIUseTextureCheckbox(BOOL& textureExistsBool)
	{
		if (ImGui::Checkbox("Use", (bool*)&textureExistsBool))
		{
			PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
			m_materialCBuffer.update(&materialData);
		}
	}

public:
	Material()
	{
		m_deviceContext			= nullptr;
		m_diffuseTexture		= nullptr;
		m_specularTexture		= nullptr;
		m_normalTexture			= nullptr;
		m_displacementTexture	= nullptr;
		m_diffTextureExists		= false;
		m_specTextureExists		= false;
		m_normTextureExists		= false;
		m_useEmisson = false;
		m_displacementExists	= false;

		m_emissiveColor = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

		m_fileDialog.SetTitle("Load Texture");
		m_fileDialog.SetTypeFilters({ ".png", ".jpg", ".jpeg", ".tga", ".dds", ".DDS", ".bmp" });
		m_texTypeToLoad = PhongTexturesTypes::NONE;
	}

	bool m_diffTextureExists;
	bool m_specTextureExists;
	bool m_normTextureExists;
	bool m_useEmisson;
	bool m_displacementExists;

	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, PS_MATERIAL_BUFFER material, TexturePaths texturePaths)
	{
		// Device
		m_deviceContext = deviceContext;

		// Material data
		m_materialData.emissive = material.emissive;
		m_materialData.ambient = material.ambient;
		m_materialData.diffuse = material.diffuse;
		m_materialData.specular = material.specular;
		m_materialData.shininess = material.shininess;

		// Textures
		loadTextures(texturePaths);

		// Constant Buffer
		m_materialCBuffer.initialize(device, m_deviceContext, &m_materialData, BufferType::CONSTANT);
	}

	// Getters
	std::string getName()
	{
		return m_name;
	}

	bool diffTextureLoaded()
	{
		return m_diffuseTexture != nullptr;
	}
	bool specTextureLoaded()
	{
		return m_specularTexture != nullptr;
	}
	bool normTextureLoaded()
	{
		return m_normalTexture != nullptr;
	}
	bool dispTextureLoaded()
	{
		return m_displacementTexture != nullptr;
	}

	// Setters
	void setMaterial(PS_MATERIAL_BUFFER material)
	{
		m_materialData = material;
		if (m_diffuseTexture != nullptr)
			m_materialData.diffTextureExists = true;
		if (m_specularTexture != nullptr)
			m_materialData.specTextureExists = true;
		if (m_normalTexture != nullptr)
			m_materialData.normTextureExists = true;
		
		PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
		m_materialCBuffer.update(&materialData);
	}
	void setTextures(TexturePaths texturePaths)
	{
		loadTextures(texturePaths);
		PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
		m_materialCBuffer.update(&materialData);
	}
	void setName(std::string newName)
	{
		m_name = newName;
	}

	// Update
	void updateUI()
	{
		float imageSize = 50;
		float imageOffset = ImGui::GetWindowSize().x - ImGui::GetContentRegionMax().x + imageSize + 40;
		UINT nameSize = 64;
		WCHAR name[64];

		if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			_bstr_t nameCStr;
			// Diffuse
			if (ImGui::TreeNodeEx((void*)"Diffuse", ImGuiTreeNodeFlags_DefaultOpen, "Diffuse"))
			{
				if (m_diffTextureExists)
				{
					ImGui::Image(m_diffuseTexture, ImVec2(imageSize, imageSize));
					m_diffuseTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
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
					m_texTypeToLoad = PhongTexturesTypes::DIFFUSE;
				}
				if (m_diffTextureExists)
					UIUseTextureCheckbox(m_materialData.diffTextureExists);

				if (ImGui::ColorEdit4("Color##2f", &m_materialData.diffuse.x, ImGuiColorEditFlags_Float))
				{
					PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}

				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Normal
			if (ImGui::TreeNodeEx((void*)"Normal", ImGuiTreeNodeFlags_DefaultOpen, "Normal"))
			{
				if (m_normTextureExists)
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
				ImGui::BeginGroup();
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_texTypeToLoad = PhongTexturesTypes::NORMAL;
				}

				if (m_normTextureExists)
					UIUseTextureCheckbox(m_materialData.normTextureExists);

				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Specular
			if (ImGui::TreeNodeEx((void*)"Specular", ImGuiTreeNodeFlags_DefaultOpen, "Specular"))
			{
				if (m_specTextureExists)
				{
					ImGui::Image(m_specularTexture, ImVec2(imageSize, imageSize));
					m_specularTexture->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameSize, (void*)name);
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
					m_texTypeToLoad = PhongTexturesTypes::NORMAL;
				}

				if (m_specTextureExists)
					UIUseTextureCheckbox(m_materialData.specTextureExists);

				if (ImGui::ColorEdit4("Color##2f", &m_materialData.specular.x, ImGuiColorEditFlags_Float))
				{
					PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}

				ImGui::EndGroup();
				ImGui::TreePop();
			}

			// Emissive
			if (ImGui::TreeNodeEx((void*)"Emissive", ImGuiTreeNodeFlags_DefaultOpen, "Emissive"))
			{
				if (ImGui::Checkbox("Use", &m_useEmisson))
				{
					if (m_useEmisson)
					{
						m_materialData.emissive = m_emissiveColor;
						PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
						m_materialCBuffer.update(&materialData);
					}
					else
					{
						m_materialData.emissive = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
						PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
						m_materialCBuffer.update(&materialData);
					}
				}
				
				if (m_useEmisson && ImGui::ColorEdit4("Color##2f", &m_emissiveColor.x, ImGuiColorEditFlags_Float))
				{
					m_materialData.emissive = m_emissiveColor;
					PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
					m_materialCBuffer.update(&materialData);
				}

				ImGui::TreePop();
			}
				
			// Displacement
			if (ImGui::TreeNodeEx((void*)"Displacement", ImGuiTreeNodeFlags_DefaultOpen, "Displacement"))
			{
				if (m_displacementExists)
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
					m_texTypeToLoad = PhongTexturesTypes::DISPLACEMENT;
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
			case PhongTexturesTypes::DIFFUSE:
				m_diffuseTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_diffuseTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				m_diffTextureExists = true;
				m_materialData.diffTextureExists = true;
				break;
			case PhongTexturesTypes::SPECULAR:
				m_specularTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_specularTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				m_specTextureExists = true;
				m_materialData.specTextureExists = true;
				break;
			case PhongTexturesTypes::NORMAL:
				m_normalTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_normalTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				m_normTextureExists = true;
				m_materialData.normTextureExists = true;
				break;
			case PhongTexturesTypes::DISPLACEMENT:
				m_displacementTexture = ResourceHandler::getInstance().getTexture(path.c_str());
				m_displacementTexture->SetPrivateData(WKPDID_D3DDebugObjectNameW, 64, path.c_str());
				m_displacementExists = true;
				break;
			default:
				break;
			}
			m_texTypeToLoad = PhongTexturesTypes::NONE; // Reset
			PS_MATERIAL_BUFFER* materialData = new PS_MATERIAL_BUFFER(m_materialData);
			m_materialCBuffer.update(&materialData);

			m_fileDialog.ClearSelected();
		}
	}

	// Render
	void sendCBufferAndTextures()
	{
		// Pixel Shader: Slot 0, Light buffer is set to Slot 2, Camera Position buffer is set to Slot 1
		m_deviceContext->PSSetConstantBuffers(0, 1, m_materialCBuffer.GetAddressOf());

		// Testures
		if (m_materialData.diffTextureExists)
			m_deviceContext->PSSetShaderResources(0, 1, &m_diffuseTexture);

		if (m_materialData.specTextureExists)
			m_deviceContext->PSSetShaderResources(1, 1, &m_specularTexture);

		if (m_materialData.normTextureExists)
			m_deviceContext->PSSetShaderResources(2, 1, &m_normalTexture);

		if (m_displacementExists)
			m_deviceContext->DSSetShaderResources(0, 1, &m_displacementTexture);
	}
};

#endif // !MATERIAL_H