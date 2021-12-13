#ifndef MESH_H
#define MESH_H

#include "pch.h"
#include "Material.h"
#include "MaterialPBR.h"

template<class T>
class Mesh
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Name
	std::string m_name;
	
	// Buffers
	std::shared_ptr< Buffer<T> > m_vertexBuffer;
	Buffer<UINT> m_IndexBuffer;
	bool m_hasIndices = false;

	// Material
	ShaderStates m_materialType;
	Material m_material;
	MaterialPBR m_materialPBR;

public:
	Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<T>& vertices, std::vector<UINT>& indices, PS_MATERIAL_BUFFER material, TexturePaths texturePaths, std::string name = "")
	{
		m_deviceContext = deviceContext;
		
		setName(name);

		m_vertexBuffer = std::make_shared< Buffer<T> >();
		m_vertexBuffer->initialize(device, deviceContext, vertices.data(), BufferType::VERTEX, (UINT)vertices.size());
		m_hasIndices = false;
		
		if (indices.size() > 0)
		{
			m_IndexBuffer.initialize(device, deviceContext, indices.data(), BufferType::INDEX, (UINT)indices.size());
			m_hasIndices = true;
		}

		// Material
		m_materialType = ShaderStates::PHONG;
		m_material.initialize(device, deviceContext, material, texturePaths);

		// PBR conversion, not accurate at all
		PS_MATERIAL_PBR_BUFFER materialPBR;
		materialPBR.albedo = XMFLOAT3(material.diffuse.x, material.diffuse.y, material.diffuse.z);
		XMVECTOR specVector = XMLoadFloat4(&material.specular);
		materialPBR.metallic = DirectX::XMVector3Length(specVector).m128_f32[0];
		materialPBR.roughness = std::pow(1 - material.shininess, 2.f);
		XMVECTOR emVector = XMLoadFloat4(&material.emissive);
		materialPBR.emissiveStrength = DirectX::XMVector3Length(emVector).m128_f32[0];

		TexturePathsPBR texturePathsPBR;
		texturePathsPBR.albedoPath = texturePaths.diffusePath;
		texturePathsPBR.normalPath = texturePaths.normalPath;
		texturePathsPBR.displacementPath = texturePaths.displacementPath;
		m_materialPBR.initialize(device, deviceContext, materialPBR, texturePathsPBR);
	}
	Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<T>& vertices, std::vector<UINT>& indices, PS_MATERIAL_PBR_BUFFER material, TexturePathsPBR texturePaths)
	{
		m_deviceContext = deviceContext;

		m_vertexBuffer = std::make_shared< Buffer<T> >();
		m_vertexBuffer->initialize(device, deviceContext, vertices.data(), BufferType::VERTEX, vertices.size());
		m_hasIndices = false;

		if (indices.size() > 0)
		{
			m_IndexBuffer.initialize(device, deviceContext, indices.data(), BufferType::INDEX, indices.size());
			m_hasIndices = true;
		}

		// Material
		m_materialType = ShaderStates::PBR;
		m_materialPBR.initialize(device, deviceContext, material, texturePaths);

		// PBR conversion, not accurate at all
		PS_MATERIAL_BUFFER materialPhong;
		materialPhong.diffuse = XMFLOAT4(material.albedo.x, material.albedo.y, material.albedo.z, 1.f);
		materialPhong.ambient = XMFLOAT4(materialPhong.diffuse.x / 10.f, materialPhong.diffuse.y / 10.f, materialPhong.diffuse.z / 10.f, 1.f);
		materialPhong.specular = XMFLOAT4(material.metallic, material.metallic, material.metallic, 1.f);
		materialPhong.shininess = 1 - std::pow(material.roughness, 1 / 2.f);
		materialPhong.emissive = XMFLOAT4(material.emissiveStrength, material.emissiveStrength, material.emissiveStrength, 1.f);

		TexturePaths texturePathsPhong;
		texturePathsPhong.diffusePath = texturePaths.albedoPath;
		texturePathsPhong.normalPath = texturePaths.normalPath;
		texturePathsPhong.displacementPath = texturePaths.displacementPath;
		m_materialPBR.initialize(device, deviceContext, materialPhong, texturePathsPhong);
	}

	Mesh(const Mesh<T>& otherMesh)
	{
		m_deviceContext = otherMesh.m_deviceContext;
		m_vertexBuffer = otherMesh.m_vertexBuffer;
		m_IndexBuffer = otherMesh.m_IndexBuffer;
		m_hasIndices = otherMesh.m_hasIndices;
		m_materialType = otherMesh.m_materialType;
		m_material = otherMesh.m_material;
		m_materialPBR = otherMesh.m_materialPBR;
		m_name = otherMesh.m_name;
	}

	// Getters
	Material& getMaterial()
	{
		switch (m_materialType)
		{
		case PHONG:
			return m_material;
			break;
		case PBR:
			return m_materialPBR;
			break;
		default:
			break;
		}
		return m_material;
	}
	std::string getName() const
	{
		return m_name;
	}

	// Setters
	void setName(std::string name)
	{
		m_name = name;
		m_material.setName(name + "_mat");
		m_materialPBR.setName(name + "_mat");
	}

	void setShaderState(ShaderStates shaderState)
	{
		m_materialType = shaderState;
	}

	void setMaterial(PS_MATERIAL_BUFFER material)
	{
		m_materialType = ShaderStates::PHONG;
		m_material.setMaterial(material);
	}

	void setMaterial(PS_MATERIAL_PBR_BUFFER material)
	{
		m_materialType = ShaderStates::PBR;
		m_materialPBR.setMaterial(material);
	}

	void setTextures(TexturePaths textures)
	{
		m_materialType = ShaderStates::PHONG;
		m_material.setTextures(textures);
	}

	void setTextures(TexturePathsPBR textures)
	{
		m_materialType = ShaderStates::PBR;
		m_materialPBR.setTextures(textures);
	}
	
	// Update
	void updateUI()
	{
		if (ImGui::TreeNodeEx(m_name.c_str()))
		{
			switch (m_materialType)
			{
			case PHONG:
				m_material.updateUI();
				break;
			case PBR:
				m_materialPBR.updateUI();
				break;
			default:
				break;
			}
			ImGui::TreePop();
		}
	}

	// Save Mesh Data
	void fillMeshData(MeshData* meshData)
	{
		meshData->name = m_name;
		meshData->matType = m_materialType;

		switch (m_materialType)
		{
		case PHONG:
			m_material.fillMaterialData(&meshData->matPhong);
			break;
		case PBR:
			m_materialPBR.fillMaterialData(&meshData->matPBR);
			break;
		default:
			break;
		}
	}

	// Render
	void render()
	{
		// Vertex Buffer
		UINT vertexOffset = 0;
		m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer->GetAddressOf(), m_vertexBuffer->getStridePointer(), &vertexOffset);

		switch (m_materialType)
		{
		case PHONG:
			m_material.sendCBufferAndTextures();
			break;
		case PBR:
			m_materialPBR.sendCBufferAndTextures();
			break;
		default:
			break;
		}

		// Draw
		if (m_hasIndices)
		{
			m_deviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
			m_deviceContext->DrawIndexed(m_IndexBuffer.getSize(), 0, 0);
		}
		else
			m_deviceContext->Draw(m_vertexBuffer->getSize(), 0);
	}
};

#endif // !MESH_H