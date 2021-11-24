#include "pch.h"
#include "ShadowMapInstance.h"

ShadowMapInstance::ShadowMapInstance()
{
	m_deviceContext = nullptr;
	m_width = 0;
	m_height = 0;
}

ShadowMapInstance::~ShadowMapInstance() {}

void ShadowMapInstance::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT width, UINT height)
{
	// Device
	m_deviceContext = deviceContext;

	// Dimensions
	m_width = width;
	m_height = height;

	// Viewport
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = (float)m_width;
	m_viewport.Height = (float)m_height;
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;

	// Resources
	// Texture 2D
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, 0, &m_shadowMapTexture);
	assert(SUCCEEDED(hr) && "Error, failed to create shadow map texture!");

	// Depth Stencil View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(m_shadowMapTexture.Get(), &depthStencilViewDesc, m_shadowMapDSV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, failed to create shadow map depth stencil view!");

	// Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
	assert(SUCCEEDED(hr) && "Error, failed to create shadow map depth stencil state!");

	// Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(m_shadowMapTexture.Get(), &shaderResourceViewDesc, m_shadowMapSRV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, failed to create shadow map shader resource view!");

	// Pipeline States

	// Sampler
	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.f;
	comparisonSamplerDesc.BorderColor[1] = 1.f;
	comparisonSamplerDesc.BorderColor[2] = 1.f;
	comparisonSamplerDesc.BorderColor[3] = 1.f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

	hr = device->CreateSamplerState(&comparisonSamplerDesc, m_comparisonSampler.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error when creating shadow map sampler state!");

	// Rasterizer
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.DepthBias = 5000;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 4.f;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	hr = device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, failed to create shadow map rasterizer state!");

	// Shaders
	ShaderFiles shaderFiles;
	shaderFiles.vs = L"ShadowMapVS.hlsl";
	shaderFiles.ps = L"ShadowMapPS.hlsl";
	m_shadowMapShaders.initialize(device, deviceContext, shaderFiles);

	// World Bounding Sphere
	m_worldBoundingSphere.Center = { 0.f, 0.f, 0.f };
	m_worldBoundingSphere.Radius = 40.f;
	//m_zOffset = -m_worldBoundingSphere.Radius;
	m_zOffset = 0;

	// Constant Buffers
	m_lightMatrixCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);
	m_invLightVpMatrixCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);
	m_shadowTextureMatrixCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);
}

Light ShadowMapInstance::getLight() const
{
	return m_directionalLight;
}

XMMATRIX ShadowMapInstance::getInvLightViewMatrix() const
{
	return m_invLightViewMatrix;
}

XMMATRIX ShadowMapInstance::getInvLightProjectionMatrix() const
{
	return m_invLightProjectionMatrix;
}

XMVECTOR ShadowMapInstance::getLightPosition() const
{
	return m_lightPosition;
}

XMVECTOR ShadowMapInstance::getLightDirection() const
{
	return XMLoadFloat3(&m_directionalLight.direction);
}

XMVECTOR ShadowMapInstance::getLightRotation() const
{
	return XMLoadFloat3(&m_lightRotationRad);
}

float ShadowMapInstance::getLightShadowRadius() const
{
	return m_worldBoundingSphere.Radius;
}

void ShadowMapInstance::buildLightMatrix(Light directionalLight, XMFLOAT3 rotationRad, XMFLOAT3 centerPosition)
{
	m_directionalLight = directionalLight;
	m_worldBoundingSphere.Center = centerPosition;
	m_worldBoundingSphere.Center.z += m_zOffset;
	m_worldBoundingSphere.Center.y = 0;

	m_lightRotationRad = rotationRad;

	// Light View Matrix
	VS_SHADOW_C_BUFFER* lightMatrices = new VS_SHADOW_C_BUFFER();
	VS_SHADOW_C_BUFFER* invLightMatrices = new VS_SHADOW_C_BUFFER();
	XMVECTOR lightDirection = XMLoadFloat3(&directionalLight.direction);
	XMVECTOR position = XMLoadFloat3(&m_worldBoundingSphere.Center);
	position = XMVectorSetW(position, 1.f);
	m_lightPosition = (-4.f * m_worldBoundingSphere.Radius * lightDirection) + position;
	
	XMVECTOR lookAt = position;
	XMVECTOR up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

	lightMatrices->lightViewMatrix = XMMatrixTranspose(XMMatrixLookAtLH(m_lightPosition, lookAt, up));
	invLightMatrices->lightViewMatrix = XMMatrixInverse(nullptr, lightMatrices->lightViewMatrix);

	// Transform World Bounding Sphere to Light Local View Space
	XMFLOAT3 worldSphereCenterLightSpace;
	XMStoreFloat3(&worldSphereCenterLightSpace, DirectX::XMVector3TransformCoord(lookAt, lightMatrices->lightViewMatrix));

	// Construct Orthographic Frustum in Light View Space
	float l = worldSphereCenterLightSpace.x - m_worldBoundingSphere.Radius;
	float b = worldSphereCenterLightSpace.y - m_worldBoundingSphere.Radius;
	//float n = worldSphereCenterLightSpace.z - m_worldBoundingSphere.Radius;
	float n = 0.f;
	float r = worldSphereCenterLightSpace.x + m_worldBoundingSphere.Radius;
	float t = worldSphereCenterLightSpace.y + m_worldBoundingSphere.Radius;
	//float f = worldSphereCenterLightSpace.z + m_worldBoundingSphere.Radius;
	float f = m_worldBoundingSphere.Radius * 6.f;

	// Local Projection Matrix
	lightMatrices->lightProjectionMatrix = XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f));
	invLightMatrices->lightProjectionMatrix = XMMatrixInverse(nullptr, lightMatrices->lightProjectionMatrix);

	// Shadow Texture Space Transformation
	XMMATRIX textureSpaceMatrix
	(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);
	XMMATRIX* textureTransformMatrix = new XMMATRIX(lightMatrices->lightViewMatrix * lightMatrices->lightProjectionMatrix * XMMatrixTranspose(textureSpaceMatrix));

	//lightMatrices->lightProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 100.f);
	//// Point light at (20, 15, 20), pointed at the origin. POV up-vector is along the y-axis.
	//XMVECTOR position = { 0.f, 0.f, -1.0f, 1.0f };
	//XMVECTOR lookAt = { 0.0f, 0.0f, 1.0f, 0.0f };
	//XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
	//lookAt += position;
	//lightMatrices->lightViewMatrix = XMMatrixLookAtLH(position, lookAt, up);
	//lightMatrices->textureTransformMatrix = XMMatrixIdentity();

	XMMATRIX inverseVpMatrix = XMMatrixInverse(nullptr, lightMatrices->lightViewMatrix) * XMMatrixInverse(nullptr, lightMatrices->lightProjectionMatrix);

	// Update data
	m_lightMatrixCBuffer.update(&lightMatrices);
	m_invLightVpMatrixCBuffer.update(&inverseVpMatrix);
	m_shadowTextureMatrixCBuffer.update(&textureTransformMatrix);
}

void ShadowMapInstance::buildLightMatrix(XMFLOAT3 centerPosition)
{
	m_worldBoundingSphere.Center = centerPosition;
	m_worldBoundingSphere.Center.z += m_zOffset;
	m_worldBoundingSphere.Center.y = 0;

	// Light View Matrix
	VS_SHADOW_C_BUFFER* lightMatrices = new VS_SHADOW_C_BUFFER();
	VS_SHADOW_C_BUFFER* invLightMatrices = new VS_SHADOW_C_BUFFER();
	XMVECTOR lightDirection = XMLoadFloat3(&m_directionalLight.direction);
	XMVECTOR position = XMLoadFloat3(&m_worldBoundingSphere.Center);
	position = XMVectorSetW(position, 1.f);
	m_lightPosition = (-2.f * m_worldBoundingSphere.Radius * lightDirection) + position;
	
	XMVECTOR lookAt = XMLoadFloat3(&m_worldBoundingSphere.Center);
	XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	lightMatrices->lightViewMatrix = XMMatrixTranspose(XMMatrixLookAtLH(m_lightPosition, lookAt, up));

	// Transform World Bounding Sphere to Light Local View Space
	XMFLOAT3 worldSphereCenterLightSpace;
	XMStoreFloat3(&worldSphereCenterLightSpace, XMVector3TransformCoord(lookAt, lightMatrices->lightViewMatrix));

	// Construct Orthographic Frustum in Light View Space
	float l = worldSphereCenterLightSpace.x - m_worldBoundingSphere.Radius;
	float b = worldSphereCenterLightSpace.y - m_worldBoundingSphere.Radius;
	float n = worldSphereCenterLightSpace.z - m_worldBoundingSphere.Radius;
	float r = worldSphereCenterLightSpace.x + m_worldBoundingSphere.Radius;
	float t = worldSphereCenterLightSpace.y + m_worldBoundingSphere.Radius;
	float f = worldSphereCenterLightSpace.z + m_worldBoundingSphere.Radius;

	// Local Projection Matrix
	lightMatrices->lightProjectionMatrix = XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f));
	invLightMatrices->lightProjectionMatrix = XMMatrixInverse(nullptr, lightMatrices->lightProjectionMatrix);
	XMMATRIX inverseVpMatrix = XMMatrixInverse(nullptr, lightMatrices->lightViewMatrix) * XMMatrixInverse(nullptr, lightMatrices->lightProjectionMatrix);

	// Update data
	m_lightMatrixCBuffer.update(&lightMatrices);
	m_invLightVpMatrixCBuffer.update(&inverseVpMatrix);
}

void ShadowMapInstance::updateLight(Light directionalLight) // Does not update matrices
{
	m_directionalLight = directionalLight;
}

ID3D11ShaderResourceView* const* ShadowMapInstance::getShadowMapSRV()
{
	return m_shadowMapSRV.GetAddressOf();
}

ID3D11ShaderResourceView* ShadowMapInstance::getShadowMapSRVNoneConst()
{
	return m_shadowMapSRV.Get();
}

ID3D11Buffer* const* ShadowMapInstance::getShadowMatrixConstantBuffer() const
{
	return m_shadowTextureMatrixCBuffer.GetAddressOf();
}

void ShadowMapInstance::clearShadowMap()
{
	m_deviceContext->ClearDepthStencilView(m_shadowMapDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void ShadowMapInstance::bindInverseVpMatrixVS()
{
	m_deviceContext->VSSetConstantBuffers(1, 1, m_invLightVpMatrixCBuffer.GetAddressOf()); // for Displacement/Volumetric Sun Scattering
}

void ShadowMapInstance::bindLightMatrixPS()
{
	m_deviceContext->PSSetConstantBuffers(2, 1, m_lightMatrixCBuffer.GetAddressOf());
}

void ShadowMapInstance::bindViewsAndRenderTarget()
{
	ID3D11ShaderResourceView* shaderResourceNullptr = nullptr;
	m_deviceContext->PSSetShaderResources(3, 1, &shaderResourceNullptr);

	m_deviceContext->OMSetRenderTargets(1, m_rendertarget, m_shadowMapDSV.Get());
	m_deviceContext->ClearDepthStencilView(m_shadowMapDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_deviceContext->RSSetState(m_rasterizerState.Get());
	m_deviceContext->RSSetViewports(1, &m_viewport);

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
	m_deviceContext->VSSetConstantBuffers(1, 1, m_lightMatrixCBuffer.GetAddressOf());
	m_deviceContext->PSSetSamplers(2, 1, m_comparisonSampler.GetAddressOf());

	m_shadowMapShaders.setShaders();
}