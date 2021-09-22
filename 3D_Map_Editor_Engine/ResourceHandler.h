#include"pch.h"
#ifndef RESOURCEHANDLER_H
#define RESOURCEHANDLER_H

#include "WICTextureLoader.h"

class ResourceHandler
{
private:
	ResourceHandler() {};
	// Device
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;

	// Textures
	std::map<const std::wstring, ID3D11ShaderResourceView*> m_textures;

	const std::wstring rootTexturePath = L"Textures\\";

	// Helper Functions
	bool createTextureFromFile(const WCHAR* texturePath, bool isCubeMap = false)
	{
		HRESULT hr;
		bool couldLoad = true;
		m_textures[texturePath] = nullptr;
		std::wstring path = texturePath;
		path = rootTexturePath + path;

		size_t i = path.rfind('.', path.length());
		std::wstring fileExtension = path.substr(i + 1, path.length() - i);
		if (fileExtension == L"dds" || fileExtension == L"DDS")
			hr = CreateDDSTextureFromFile(m_device, m_deviceContext, path.c_str(), nullptr, &m_textures[texturePath]);
		else if (fileExtension == L"hdr")
		{
			DirectX::TexMetadata metadata;
			DirectX::ScratchImage scratchImage;
			hr = LoadFromHDRFile(path.c_str(), &metadata, scratchImage);
			assert(SUCCEEDED(hr) && "Error, failed to load HDR texture file!");

			hr = CreateShaderResourceView(m_device, scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, &m_textures[texturePath]);
		}
		else if (fileExtension == L"tga" || fileExtension == L"TGA")
		{
			DirectX::TexMetadata metadata;
			DirectX::ScratchImage scratchImage;
			DirectX::LoadFromTGAFile(path.c_str(), &metadata, scratchImage);
			DirectX::ScratchImage mipChain;

			hr = GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(),
				scratchImage.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChain);
			assert(SUCCEEDED(hr) && "Error, failed to generate mipmaps for TGA texture file!");

			hr = CreateShaderResourceView(m_device, mipChain.GetImages(), mipChain.GetImageCount(), metadata, &m_textures[texturePath]);
		}
		else
			hr = CreateWICTextureFromFile(m_device, m_deviceContext, path.c_str(), nullptr, &m_textures[texturePath]);
		

		if (FAILED(hr))
		{
			OutputDebugString(L"Failed to load: ");
			OutputDebugString(texturePath);
			OutputDebugString(L"\n");
			couldLoad = false;
		}
		else
		{
			OutputDebugString(L"Texture loaded: ");
			OutputDebugString(texturePath);
			OutputDebugString(L"\n");
		}
		assert(SUCCEEDED(hr) && "Error, failed to load texture file!");

		return couldLoad;
	}

public:
	ResourceHandler(ResourceHandler const&) = delete;
	void operator=(ResourceHandler const&) = delete;
	~ResourceHandler()
	{
		for (auto& texture : m_textures)
			texture.second->Release();
	}
	static ResourceHandler& getInstance()
	{
		static ResourceHandler handlerInstance;
		return handlerInstance;
	}

	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		m_device = device;
		m_deviceContext = deviceContext;
	}

	ID3D11ShaderResourceView* getTexture(const WCHAR* texturePath, bool isCubeMap = false)
	{
		std::wstring path(texturePath);
		if (path.find(rootTexturePath) == 0)
			path.erase(0, rootTexturePath.length());

		if (!m_textures.count(path.c_str()))
		{
			if (m_device != nullptr)
				createTextureFromFile(path.c_str(), isCubeMap);

			return m_textures[path.c_str()];
		}
		else
			return m_textures[path.c_str()];
	}

	/*const uint8_t* getBytesFromImage(const WCHAR* texturePath)
	{
		if (!m_textures.count(texturePath))
			return new uint8_t();

		const uint8_t* byteImage;
		ID3D11Resource* recource;
		m_textures[texturePath]->GetResource(&recource);
		return recource.;
	}*/
};

#endif // !RESOURCEHANDLER_H