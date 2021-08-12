#ifndef MESH_H
#define MESH_H

#include "pch.h"
#include "Buffer.h"
#include "Material.h"

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
	Material m_material;

public:
	Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<T>& vertices, std::vector<UINT>& indices, PS_MATERIAL_BUFFER material, TexturePaths texturePaths)
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

		m_material.initialize(device, deviceContext, material, texturePaths);
	}

	Mesh(const Mesh<T>& otherMesh)
	{
		m_deviceContext = otherMesh.m_deviceContext;
		m_vertexBuffer = otherMesh.m_vertexBuffer;
		m_IndexBuffer = otherMesh.m_IndexBuffer;
		m_hasIndices = otherMesh.m_hasIndices;
		m_material = otherMesh.m_material;
	}

	Material& getMaterial()
	{
		return m_material;
	}

	void setName(std::string name)
	{
		m_name = name;
		m_material.setName(name + "_mat");
	}

	void setMaterial(PS_MATERIAL_BUFFER material)
	{
		m_material.setMaterial(material);
	}

	void setTextures(TexturePaths textures)
	{
		m_material.setTextures(textures);
	}

	void render()
	{
		// Vertex Buffer
		UINT vertexOffset = 0;
		m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer->GetAddressOf(), m_vertexBuffer->getStridePointer(), &vertexOffset);

		m_material.sendCBufferAndTextures();

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