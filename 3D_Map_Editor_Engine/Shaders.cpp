#include "pch.h"
#include "Shaders.h"

Shaders::Shaders()
{
	m_device = nullptr;
	m_deviceContext = nullptr;

	m_topology = D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

Shaders::~Shaders() {}


void Shaders::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ShaderFiles names, LayoutType layoutType, D3D_PRIMITIVE_TOPOLOGY topology, bool streamOutput)
{
	// Device
	m_device = device;
	m_deviceContext = deviceContext;

	// Topology
	m_topology = topology;

	// Layout Type
	m_layoutType = layoutType;

	// Files
	m_files = names;

	// Error Blob
	ID3DBlob* errorBlob = nullptr;
	
	// Helper Varables
	HRESULT hr;
	LPCWSTR path = L"";

	// Flags
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
	#endif

	// Create Vertex Shader
	if (names.vs != L"")
	{
		std::wstring temp = std::wstring(L"Shaders\\") + names.vs;
		path = temp.c_str();

		ID3DBlob* vsBlob = nullptr;
		hr = D3DCompileFromFile(
			path,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"vs_5_0",
			flags,
			0,
			&vsBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (vsBlob)
				vsBlob->Release();
			OutputDebugStringA("Error, Vertex shaders could not be compiled!");
			//assert(SUCCEEDED(hr) && "Error, Vertex shaders could not be created!");
		}
		else
		{
			if (m_vertexShader.Get())
				m_vertexShader.ReleaseAndGetAddressOf();

			hr = m_device->CreateVertexShader(
				vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),
				nullptr,
				&m_vertexShader
			);

			assert(SUCCEEDED(hr) && "Error, Vertex shaders could not be created!");

			// Vertex Layout
			if (layoutType == LayoutType::POS_NOR_TEX) {
				hr = device->CreateInputLayout(
					VertexPosNormTexDesc,
					VertexPosNormTexElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			else if (layoutType == LayoutType::POS_TEX_FINDEX) {
				hr = device->CreateInputLayout(
					VertexPosTexFrustumIndexDesc,
					VertexPosTexFrustumIndexElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			else if (layoutType == LayoutType::POS_TEX)
			{
				hr = device->CreateInputLayout(
					VertexPosTexDesc,
					VertexPosTexElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			else if (layoutType == LayoutType::POS_COL)
			{
				hr = device->CreateInputLayout(
					VertexPosColDesc,
					VertexPosColElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			else if (layoutType == LayoutType::POS)
			{
				hr = device->CreateInputLayout(
					VertexPosDesc,
					VertexPosElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			else if (layoutType == LayoutType::POS_NOR_TEX_TAN)
			{
				hr = device->CreateInputLayout(
					VertexPosNormTexTanDesc,
					VertexPosNormTexTanElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			else if (layoutType == LayoutType::PARTICLE)
			{
				hr = device->CreateInputLayout(
					VertexParticleDesc,
					VertexParticleElementCount,
					vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(),
					&m_layout
				);
			}
			assert(SUCCEEDED(hr) && "Error, Input layout could not be created!");
		}
	}


	// Create Hull Shader
	if (names.hs != L"")
	{
		std::wstring temp = std::wstring(L"Shaders\\") + names.hs;
		path = temp.c_str();
		ID3DBlob* hsBlob = nullptr;
		if (errorBlob) errorBlob->Release();
		errorBlob = nullptr;

		hr = D3DCompileFromFile(
			path,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"hs_5_0",
			flags,
			0,
			&hsBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (hsBlob)
				hsBlob->Release();
			OutputDebugStringA("Error, Hull shaders could not be compiled!");
			//assert(SUCCEEDED(hr) && "Error, Hull shaders could not be created!");
		}
		else
		{
			if (m_hullShader.Get())
				m_hullShader.ReleaseAndGetAddressOf();

			hr = m_device->CreateHullShader(
				hsBlob->GetBufferPointer(),
				hsBlob->GetBufferSize(),
				nullptr,
				&m_hullShader
			);
			assert(SUCCEEDED(hr) && "Error, Hull shaders could not be created!");
			hsBlob->Release();
		}
	}


	// Create Domain Shader
	if (names.ds != L"")
	{
		std::wstring temp = std::wstring(L"Shaders\\") + names.ds;
		path = temp.c_str();
		ID3DBlob* dsBlob = nullptr;
		if (errorBlob) errorBlob->Release();
		errorBlob = nullptr;

		hr = D3DCompileFromFile(
			path,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"ds_5_0",
			flags,
			0,
			&dsBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (dsBlob)
				dsBlob->Release();
			OutputDebugStringA("Error, Domain shaders could not be compiled!");
			//assert(SUCCEEDED(hr) && "Error, Domain shaders could not be created!");
		}
		else
		{
			if (m_domainShader.Get())
				m_domainShader.ReleaseAndGetAddressOf();

			hr = m_device->CreateDomainShader(
				dsBlob->GetBufferPointer(),
				dsBlob->GetBufferSize(),
				nullptr,
				&m_domainShader
			);
			assert(SUCCEEDED(hr) && "Error, Domain shaders could not be created!");
			dsBlob->Release();
		}
	}


	// Create Geometry Shader
	if (names.gs != L"")
	{
		std::wstring temp = std::wstring(L"Shaders\\") + names.gs;
		path = temp.c_str();
		ID3DBlob* gsBlob = nullptr;
		if (errorBlob) errorBlob->Release();
		errorBlob = nullptr;

		hr = D3DCompileFromFile(
			path,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"gs_5_0",
			flags,
			0,
			&gsBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (gsBlob)
				gsBlob->Release();
			OutputDebugStringA("Error, Geometry shaders could not be compiled!");
			//assert(SUCCEEDED(hr) && "Error, Geometry shaders could not be created!");
		}
		else
		{
			if (m_geometryShader.Get())
				m_geometryShader.ReleaseAndGetAddressOf();

			/*if (streamOutput)
			{
				m_device->CreateGeometryShaderWithStreamOutput(
					gsBlob->GetBufferPointer(),
					gsBlob->GetBufferSize(),
					VertexParticleSoDecl,
					VertexParticleElementCount,
					NULL,
					0,
					0,
					nullptr,
					&m_geometryShader);
			}
			else
			{*/
				hr = m_device->CreateGeometryShader(
					gsBlob->GetBufferPointer(),
					gsBlob->GetBufferSize(),
					nullptr,
					&m_geometryShader);
			//}

			assert(SUCCEEDED(hr) && "Error, Geometry shaders could not be created!");
			gsBlob->Release();
		}
	}


	// Create Pixel Shader
	if (names.ps != L"")
	{
		std::wstring temp = std::wstring(L"Shaders\\") + names.ps;
		path = temp.c_str();
		ID3DBlob* psBlob = nullptr;
		if (errorBlob) errorBlob->Release();
		errorBlob = nullptr;

		hr = D3DCompileFromFile(
			path,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"ps_5_0",
			flags,
			0,
			&psBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (psBlob)
				psBlob->Release();
			OutputDebugStringA("Error, Pixel shaders could not be compiled!");
			//assert(SUCCEEDED(hr) && "Error, Pixel shaders could not be created!");
		}
		else
		{
			if (m_pixelShader.Get())
				m_pixelShader.ReleaseAndGetAddressOf();

			hr = m_device->CreatePixelShader(
				psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(),
				nullptr,
				&m_pixelShader
			);
			assert(SUCCEEDED(hr) && "Error, Pixel shaders could not be created!");
			psBlob->Release();
		}
	}


	// Create Compute Shader
	if (names.cs != L"")
	{
		std::wstring temp = std::wstring(L"Shaders\\") + names.cs;
		path = temp.c_str();
		ID3DBlob* csBlob = nullptr;
		if (errorBlob) errorBlob->Release();
		errorBlob = nullptr;

		hr = D3DCompileFromFile(
			path,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"cs_5_0",
			flags,
			0,
			&csBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			if (csBlob)
				csBlob->Release();
			OutputDebugStringA("Error, Compute shaders could not be compiled!");
			//assert(SUCCEEDED(hr) && "Error, Compute shaders could not be created!");
		}
		else
		{
			if (m_computeShader.Get())
				m_computeShader.ReleaseAndGetAddressOf();

			hr = m_device->CreateComputeShader(
				csBlob->GetBufferPointer(),
				csBlob->GetBufferSize(),
				nullptr,
				&m_computeShader
			);
			assert(SUCCEEDED(hr) && "Error, Compute shaders could not be created!");
			csBlob->Release();
		}
	}
}

void Shaders::updateShaders()
{
	initialize(m_device, m_deviceContext, m_files, m_layoutType);
}

void Shaders::setShaders()
{
	m_deviceContext->IASetPrimitiveTopology(m_topology);
	m_deviceContext->IASetInputLayout(m_layout.Get());

	m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_deviceContext->HSSetShader(m_hullShader.Get(), nullptr, 0);
	m_deviceContext->DSSetShader(m_domainShader.Get(), nullptr, 0);
	m_deviceContext->GSSetShader(m_geometryShader.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	m_deviceContext->CSSetShader(m_computeShader.Get(), nullptr, 0);
}

void Shaders::unbindShaders()
{
	m_deviceContext->VSSetShader(nullptr, nullptr, 0);
	m_deviceContext->HSSetShader(nullptr, nullptr, 0);
	m_deviceContext->DSSetShader(nullptr, nullptr, 0);
	m_deviceContext->GSSetShader(nullptr, nullptr, 0);
	m_deviceContext->PSSetShader(nullptr, nullptr, 0);
	m_deviceContext->CSSetShader(nullptr, nullptr, 0);
}