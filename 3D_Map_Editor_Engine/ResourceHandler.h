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

	// Textures
	std::map<const std::wstring, ID3D11ShaderResourceView*> m_textures;

	// Helper Functions
	bool createTextureFromFile(ID3D11Device* device, const WCHAR* texturePath)
	{
		HRESULT hr;
		bool couldLoad = true;
		m_textures[texturePath] = nullptr;
		std::wstring path = texturePath;
		path = L"Textures\\" + path;

		size_t i = path.rfind('.', path.length());
		std::wstring fileExtension = path.substr(i + 1, path.length() - i);
		if (fileExtension == L"dds" || fileExtension == L"DDS")
			hr = CreateDDSTextureFromFile(device, path.c_str(), nullptr, &m_textures[texturePath]);
		else
			hr = CreateWICTextureFromFile(device, path.c_str(), nullptr, &m_textures[texturePath]);

		if (FAILED(hr))
			couldLoad = false;
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

	void setDevice(ID3D11Device* device)
	{
		m_device = device;
	}

	ID3D11ShaderResourceView* getTexture(const WCHAR* texturePath)
	{
		if (!m_textures.count(texturePath))
		{
			if (m_device != nullptr)
				createTextureFromFile(m_device, texturePath);

			return m_textures[texturePath];
		}
		else
			return m_textures[texturePath];
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