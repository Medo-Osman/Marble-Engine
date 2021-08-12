#include "pch.h"
#ifndef BUFFER_H
#define BUFFER_H


enum class BufferType { VERTEX, INDEX, CONSTANT};

template<class T>
class Buffer
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Buffer
	Microsoft::WRL::ComPtr< ID3D11Buffer > m_buffer;

	// Data
	std::shared_ptr<T> m_data;

	// Meta Data
	UINT m_stride;
	UINT m_nrOf;

public:
	Buffer()
	{
		m_deviceContext = nullptr;
		m_stride = 0;
		m_nrOf = 0;
	}
	Buffer(const Buffer<T>& otherBuffer)
	{
		m_deviceContext = otherBuffer.m_deviceContext;
		m_buffer = otherBuffer.m_buffer;
		m_data = otherBuffer.m_data;
		m_stride = otherBuffer.m_stride;
		m_nrOf = otherBuffer.m_nrOf;
	}
	Buffer<T>& operator=(const Buffer<T>& otherBuffer)
	{
		m_deviceContext = otherBuffer.m_deviceContext;
		m_buffer = otherBuffer.m_buffer;
		m_data = otherBuffer.m_data;
		m_stride = otherBuffer.m_stride;
		m_nrOf = otherBuffer.m_nrOf;
		return *this;
	}
	~Buffer() {}

	// Initialization
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, T* data, BufferType bufferType, UINT nrOfVertices = 0, bool immutable = true, bool streamOutputVertices = false)
	{
		m_deviceContext = deviceContext;
		if (data == nullptr)
			m_data = std::make_shared<T>();
		else
			m_data = std::make_shared<T>(*data);

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		if (bufferType == BufferType::CONSTANT)
		{
			// Buffer Description
			bufferDesc.ByteWidth = sizeof(T);
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			// Data
			D3D11_SUBRESOURCE_DATA constantData;
			constantData.pSysMem = m_data.get();
			constantData.SysMemPitch = 0;
			constantData.SysMemSlicePitch = 0;

			HRESULT hr = device->CreateBuffer(&bufferDesc, &constantData, m_buffer.GetAddressOf());
			assert(SUCCEEDED(hr) && "Error, failed to create constant buffer!");
		}
		else if (bufferType == BufferType::VERTEX)
		{
			// Meta Data
			m_nrOf = nrOfVertices;
			m_stride = UINT(sizeof(T));

			// Buffer Description
			bufferDesc.ByteWidth = m_stride * m_nrOf;
			if (immutable)
				bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			else
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;

			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			if (streamOutputVertices)
				bufferDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
			bufferDesc.CPUAccessFlags = 0;

			// Subresource data
			D3D11_SUBRESOURCE_DATA vertexData;
			ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
			vertexData.pSysMem = data;
			vertexData.SysMemPitch = 0;
			vertexData.SysMemSlicePitch = 0;

			HRESULT hr = device->CreateBuffer(&bufferDesc, &vertexData, m_buffer.GetAddressOf());
			assert(SUCCEEDED(hr) && "Error, failed to create Vertex buffer!");
		}
		else if (bufferType == BufferType::INDEX)
		{
			// Meta Data
			m_nrOf = nrOfVertices;
			m_stride = sizeof(UINT);

			// Buffer Description
			if (immutable)
				bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			else
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.ByteWidth = m_nrOf * m_stride;
			bufferDesc.CPUAccessFlags = 0;

			// Subresource data
			D3D11_SUBRESOURCE_DATA indexData;
			ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
			indexData.pSysMem = data;

			HRESULT hr = device->CreateBuffer(&bufferDesc, &indexData, m_buffer.GetAddressOf());
			assert(SUCCEEDED(hr) && "Error, failed to create Index buffer!");
		}
	}

	// Accessors
	ID3D11Buffer* Get() const { return m_buffer.Get(); }
	ID3D11Buffer* const* GetAddressOf() const { return m_buffer.GetAddressOf(); }

	const UINT getStride() const { return *m_stride; }
	const UINT* getStridePointer() const { return &m_stride; }

	UINT getSize() const { return m_nrOf; }

	// Update
	void update(T** data)
	{
		m_data.reset(*data);
		D3D11_MAPPED_SUBRESOURCE mapSubresource;
		HRESULT hr = m_deviceContext->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapSubresource);
		assert(SUCCEEDED(hr) && "Error, failed to map constant buffer!");
		CopyMemory(mapSubresource.pData, m_data.get(), sizeof(T));

		m_deviceContext->Unmap(m_buffer.Get(), 0);
	}
};

#endif // !BUFFER_H