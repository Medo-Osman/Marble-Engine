#include "pch.h"
#include "RenderHandler.h"

RenderHandler::RenderHandler()
{
	m_window = nullptr;
	m_settings = nullptr;
	m_wireframeMode = false;

	m_viewport = D3D11_VIEWPORT();
	m_animationDirection = 1.f;

	m_fileDialog.SetTitle("Load Texture");
	m_fileDialog.SetTypeFilters({ ".dds", ".DDS" });
}

RenderHandler::~RenderHandler()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void RenderHandler::initDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions
	m_clientWidth = winRect.right;
	m_clientHeight = winRect.bottom;

	// Create Device and Swap Chain
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = *m_window;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.Width = winRect.right;
	swapChainDesc.BufferDesc.Height = winRect.bottom;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	D3D_FEATURE_LEVEL feature_level[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_swapChain,
		&m_device,
		feature_level,
		&m_deviceContext
	);
	assert(SUCCEEDED(hr) && "Error, failed to create device and swapchain!");
}

void RenderHandler::initRenderTarget(RenderTexture& rtv, UINT width, UINT height, UINT mipLevels)
{
	// Texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = mipLevels;
	textureDesc.ArraySize = 1;
	textureDesc.Format = rtv.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if (mipLevels > 1)
		textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

	HRESULT hr = m_device->CreateTexture2D(&textureDesc, NULL, &rtv.rtt);
	assert(SUCCEEDED(hr) && "Error, render target texture could not be created!");

	// Render Rarget View
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hr = m_device->CreateRenderTargetView(rtv.rtt, &renderTargetViewDesc, &rtv.rtv);
	assert(SUCCEEDED(hr) && "Error, render target view could not be created!");

	// Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = mipLevels;

	hr = m_device->CreateShaderResourceView(rtv.rtt, &srvDesc, &rtv.srv);
	assert(SUCCEEDED(hr) && "Error, shader resource view could not be created!");

	// Unordered Access View
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	hr = m_device->CreateUnorderedAccessView(rtv.rtt, &uavDesc, &rtv.uav);
	assert(SUCCEEDED(hr) && "Error, unordered access view could not be created!");
}

void RenderHandler::initRenderTargets()
{
	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions

	// GBuffer
	
	// - Albedo Metallic
	initRenderTarget(m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC], winRect.right, winRect.bottom);
	// - Normal Roughness
	m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	initRenderTarget(m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS], winRect.right, winRect.bottom);
	// - Emissive Shadow Mask
	initRenderTarget(m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK], winRect.right, winRect.bottom);
	// - Ambient Occlusion
	initRenderTarget(m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION], winRect.right, winRect.bottom);

	// HDR Render Target
	m_hdrRTV.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	initRenderTarget(m_hdrRTV, winRect.right, winRect.bottom);

	// Output Render Target
	// - Get Back Buffer Texture for Output RenderTarget
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	assert(SUCCEEDED(hr) && "Error, failed to get backbuffer texture!");
	// - Create Render Target View from Back Buffer Tetxure
	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_outputRTV);
	assert(SUCCEEDED(hr) && "Error, failed to create ouput render target view!");
	backBuffer->Release();
}

void RenderHandler::initViewPort()
{
	RECT winRect;
	GetClientRect(*m_window, &winRect);

	m_viewport.TopLeftX = (FLOAT)winRect.left;
	m_viewport.TopLeftY = (FLOAT)winRect.top;
	m_viewport.Width = (FLOAT)winRect.right;
	m_viewport.Height = (FLOAT)winRect.bottom;
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}

void RenderHandler::initDepthStencilBuffer()
{
	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions

	// Create Depth Stencil Buffer Texture
	D3D11_TEXTURE2D_DESC dsBufferDesc;
	dsBufferDesc.Width = winRect.right;
	dsBufferDesc.Height = winRect.bottom;
	dsBufferDesc.MipLevels = 1;
	dsBufferDesc.ArraySize = 1;
	dsBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsBufferDesc.SampleDesc.Count = 1;
	dsBufferDesc.SampleDesc.Quality = 0;
	dsBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	dsBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dsBufferDesc.CPUAccessFlags = 0;
	dsBufferDesc.MiscFlags = 0;

	HRESULT hr = m_device->CreateTexture2D(&dsBufferDesc, NULL, &m_depthStencilBuffer);
	assert(SUCCEEDED(hr) && "Error, failed to create depth buffer texture!");

	// Create Depth Stencil View
	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc;
	dsViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Flags = 0;
	dsViewDesc.Texture2D.MipSlice = 0;

	hr = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsViewDesc, &m_depthStencilView);
	assert(SUCCEEDED(hr) && "Error, failed to create depth stencil view!");

	// Create Depth Stencil Shader Resource View, Pointer Saved in G-Buffer array
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = m_device->CreateShaderResourceView(m_depthStencilBuffer.Get(), &srvDesc, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv);
	assert(SUCCEEDED(hr) && "Error, failed to create depth stencil shader resource view!");

	// Create Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

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

	// Create Depth Stencil State
	hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
	assert(SUCCEEDED(hr) && "Error, failed to create depth stencil state!");
	
	// Create Read Only Depth Stencil State
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_device->CreateDepthStencilState(&dsDesc, &m_readOnlyDepthStencilState);
	assert(SUCCEEDED(hr) && "Error, failed to create depth stencil state!");

	// Create Disabled Depth Stencil State
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_device->CreateDepthStencilState(&dsDesc, &m_disabledDepthStencilState);
	assert(SUCCEEDED(hr) && "Error, failed to create disabled depth stencil state!");
}

void RenderHandler::initRenderStates()
{
	// Rasterizer States
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK; //D3D11_CULL_FRONT; // D3D11_CULL_NONE to Disable Back Face Culling
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	// Default
	HRESULT hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_defaultRasterizerState);
	assert(SUCCEEDED(hr) && "Error, failed to create default rasterizer state!");
	
	// Wireframe
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_wireframeRasterizerState);
	assert(SUCCEEDED(hr) && "Error, failed to create wireframe rasterizer state!");

	// Cull None
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_cullOffRasterizerState);
	assert(SUCCEEDED(hr) && "Error, failed to create cull none rasterizer state!");

	// Cull None Wireframe
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_cullOffWireframeRasterizerState);
	assert(SUCCEEDED(hr) && "Error, failed to create cull none rasterizer state!");

	// Sampler State Setup
	D3D11_SAMPLER_DESC samplerStateDesc;
	// - Wrap Sampler
	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.MipLODBias = 0.0f;
	samplerStateDesc.MinLOD = 0.0f;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerStateDesc.MaxAnisotropy = 1;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerStateDesc.BorderColor[0] = 1.f;
	samplerStateDesc.BorderColor[1] = 1.f;
	samplerStateDesc.BorderColor[2] = 1.f;
	samplerStateDesc.BorderColor[3] = 1.f;

	hr = m_device->CreateSamplerState(&samplerStateDesc, &m_defaultWrapSamplerState);
	assert(SUCCEEDED(hr) && "Error, failed to create default wrap sampler state!");
	m_deviceContext->PSSetSamplers(1, 1, m_defaultWrapSamplerState.GetAddressOf()); // ImGui uses slot 0
	m_deviceContext->GSSetSamplers(0, 1, m_defaultWrapSamplerState.GetAddressOf());

	// - Border Sampler
	
	// - - White Border
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerStateDesc.BorderColor[0] = 1.f;
	samplerStateDesc.BorderColor[1] = 1.f;
	samplerStateDesc.BorderColor[2] = 1.f;
	samplerStateDesc.BorderColor[3] = 1.f;
	hr = m_device->CreateSamplerState(&samplerStateDesc, &m_whiteBorderSamplerState);
	assert(SUCCEEDED(hr) && "Error, failed to create white border sampler state!");

	// - - Black Border
	samplerStateDesc.BorderColor[0] = 0.f;
	samplerStateDesc.BorderColor[1] = 0.f;
	samplerStateDesc.BorderColor[2] = 0.f;
	samplerStateDesc.BorderColor[3] = 0.f;

	hr = m_device->CreateSamplerState(&samplerStateDesc, &m_defaultBorderSamplerState);
	assert(SUCCEEDED(hr) && "Error, failed to create default border sampler state!");
	m_deviceContext->CSSetSamplers(0, 1, m_defaultBorderSamplerState.GetAddressOf());
	m_deviceContext->HSSetSamplers(0, 1, m_defaultBorderSamplerState.GetAddressOf());
	m_deviceContext->DSSetSamplers(0, 1, m_defaultBorderSamplerState.GetAddressOf());
	m_deviceContext->GSSetSamplers(1, 1, m_defaultBorderSamplerState.GetAddressOf());
	m_deviceContext->PSSetSamplers(5, 1, m_defaultBorderSamplerState.GetAddressOf());

	// - - Black Border Comparison
	samplerStateDesc.MinLOD = 0.f;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerStateDesc.MipLODBias = 0.f;
	samplerStateDesc.MaxAnisotropy = 0;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerStateDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

	hr = m_device->CreateSamplerState(&samplerStateDesc, m_blackBorderComparisonSamplerState.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, failed to creat black border comparison sampler state!");
	m_deviceContext->GSSetSamplers(2, 1, m_blackBorderComparisonSamplerState.GetAddressOf()); // Used for Lens Flare

	// Blend States
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendStateDesc, m_blendStateNoBlend.GetAddressOf());

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendStateDesc, m_blendStateAdditiveBlend.GetAddressOf());
	
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_device->CreateBlendState(&blendStateDesc, m_blendStatePreMultipliedAlphaBlend.GetAddressOf());

	blendStateDesc.RenderTarget[0].BlendEnable = false;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendStateDesc, m_blendStateBlend.GetAddressOf());
}

void RenderHandler::initSSAOBlurPass(UINT width, UINT height, DXGI_FORMAT format)
{
	ShaderFiles shaderFiles;
	shaderFiles.cs = L"EdgePreservingBlurCS.hlsl";
	m_edgePreservingBlurCS.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS);
	m_blurCData = std::make_unique< CS_BLUR_CBUFFER >();
	m_blurCData->projectionMatrix = m_camera.getProjectionMatrix();
	calculateBlurWeights(m_blurCData.get(), MAX_BLUR_RADIUS, m_ssaoBlurSigma);

	m_blurCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), m_blurCData.get(), BufferType::CONSTANT);

	// Texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* texture;
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, NULL, &texture);
	assert(SUCCEEDED(hr) && "Error, blur pass ping pong texture could not be created!");
	
	// SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = m_device->CreateShaderResourceView(texture, &srvDesc, m_blurPingPongSRV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, blur pass ping pong shader resource view could not be created!");

	// UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	hr = m_device->CreateUnorderedAccessView(texture, &uavDesc, m_blurPingPongUAV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, blur pass ping pong unordered access view could not be created!");

	texture->Release();
}

void RenderHandler::initVolumetricSunPass()
{
	// Mesh
	//std::vector<VertexPosNormTexTan> vertices;
	//std::vector<UINT> indices;

	//int gridDimension = (int)m_shadowInstance.getLightShadowRadius();
	////int gridDimension = 20;

	//float du = 1.0f / (float)gridDimension;
	//float dv = 1.0f / (float)gridDimension;
	//float centerOffset = 0.5f;//du / 2.f; if not uniform
	//float scaling = 2.f;

	//// - Vertices
	//for (int j = 0; j <= gridDimension; ++j)
	//{
	//	for (int i = 0; i <= gridDimension; ++i)
	//	{
	//		VertexPosNormTexTan vertex;
	//		float x = ((float)i * du - centerOffset) * scaling;
	//		float y = ((float)j * dv - centerOffset) * scaling;
	//		vertex.position = XMFLOAT3(-x, y, 0.0f); // X is reversed to account for shadow map perspective shift
	//		vertex.normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	//		vertex.texCoord = XMFLOAT2(1.f - (i * du), 1.f - (j * dv)); // Reverse to get the right cutout Shadowmap perspective

	//		vertices.push_back(vertex);
	//	}
	//}
	//// - - Add Cap
	//VertexPosNormTexTan vertex;
	//vertex.position = XMFLOAT3(-centerOffset * scaling		, -centerOffset * scaling, 0.0f);
	//vertex.texCoord = XMFLOAT2(0, 0);
	//vertices.push_back(vertex);

	//vertex.position = XMFLOAT3((1 - centerOffset) * scaling	, -centerOffset * scaling, 0.0f);
	//vertices.push_back(vertex);
	//
	//vertex.position = XMFLOAT3(-centerOffset * scaling		, (1 - centerOffset) * scaling, 0.0f);
	//vertices.push_back(vertex);
	//
	//vertex.position = XMFLOAT3((1 - centerOffset) * scaling	, (1 - centerOffset) * scaling, 0.0f);
	//vertices.push_back(vertex);

	//// - Indices
	//for (int j = 0; j < gridDimension; ++j)
	//{
	//	for (int i = 0; i < gridDimension; ++i)
	//	{
	//		int row1 = j * (gridDimension + 1);
	//		int row2 = (j + 1) * (gridDimension + 1);

	//		// Row 1
	//		indices.push_back(row1 + i);
	//		indices.push_back(row1 + i + 1);

	//		// Row2
	//		indices.push_back(row2 + i);
	//		indices.push_back(row2 + i + 1);

	//		//// triangle 1
	//		//indices.push_back(row1 + i);
	//		//indices.push_back(row1 + i + 1);
	//		//indices.push_back(row2 + i + 1);

	//		//// triangle 2
	//		//indices.push_back(row1 + i);
	//		//indices.push_back(row2 + i + 1);
	//		//indices.push_back(row2 + i);
	//	}
	//}

	//// - - Add Cap
	//indices.push_back(vertices.size() - 4);
	//indices.push_back(vertices.size() - 3);
	//indices.push_back(vertices.size() - 2);
	//indices.push_back(vertices.size() - 1);

	//// - Create Mesh
	//m_lightVolumeMesh = std::make_unique<Mesh<VertexPosNormTexTan>>(m_device.Get(), m_deviceContext.Get(), vertices, indices, PS_MATERIAL_BUFFER(), TexturePaths(), "Volumetric Sun Scattering Mesh");

	// Constant Buffer
	m_lightVolumeWvpCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), nullptr, BufferType::CONSTANT);
	m_lightVolumeTessCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), nullptr, BufferType::CONSTANT);
	
	// Shaders
	ShaderFiles sf;
	sf.vs = L"FullscreenQuadVS.hlsl";
	/*sf.hs = L"VolumetricSunHS.hlsl";
	sf.ds = L"VolumetricSunDS.hlsl";*/
	sf.ps = L"VolumetricSunPS.hlsl";
	m_volumetricSunShaders.initialize(m_device.Get(), m_deviceContext.Get(), sf, LayoutType::POS);

	// Render Target
	m_volumetricAccumulationRTV.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	initRenderTarget(m_volumetricAccumulationRTV, m_clientWidth, m_clientHeight);
}

void RenderHandler::initBloomPass(UINT width, UINT height)
{
	// Render Textures
	m_bloomBuffers[Base].format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	initRenderTarget(m_bloomBuffers[Base], width / 2, height / 2, NR_OF_BLOOM_MIPS);
	std::wstring texName = L"Base Bloom Buffer";
	m_bloomBuffers[Base].srv->SetPrivateData(WKPDID_D3DDebugObjectNameW, (UINT)sizeof(texName), texName.c_str());

	m_bloomBuffers[FirstPingPong].format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	initRenderTarget(m_bloomBuffers[FirstPingPong], width / 2, height / 2, NR_OF_BLOOM_MIPS);
	texName = L"FirstPingPong Bloom Buffer";
	m_bloomBuffers[FirstPingPong].srv->SetPrivateData(WKPDID_D3DDebugObjectNameW, (UINT)sizeof(texName), texName.c_str());

	m_bloomBuffers[SecondPingPong].format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	initRenderTarget(m_bloomBuffers[SecondPingPong], width / 2, height / 2, NR_OF_BLOOM_MIPS);
	texName = L"SecondPingPong Bloom Buffer";
	m_bloomBuffers[SecondPingPong].srv->SetPrivateData(WKPDID_D3DDebugObjectNameW, (UINT)sizeof(texName), texName.c_str());

	// - Mip UAVS (only one mip slice can be accesed by a UAV)
	for (size_t i = 0; i < NR_OF_BLOOM_MIPS; i++)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = (UINT)i;
		
		// - Base Buffer
		uavDesc.Format = m_bloomBuffers[Base].format;
		HRESULT hr = m_device->CreateUnorderedAccessView(m_bloomBuffers[Base].rtt, &uavDesc, m_bloomMipUAVs[Base][i].GetAddressOf());
		assert(SUCCEEDED(hr) && "Error, unordered access view could not be created!");
	
		texName = L"BaseBloomBuffer " + std::to_wstring(i) + L" UAV";
		m_bloomMipUAVs[Base][i]->SetPrivateData(WKPDID_D3DDebugObjectNameW, (UINT)sizeof(texName), texName.c_str());

		// - First Ping Pong Buffer
		uavDesc.Format = m_bloomBuffers[FirstPingPong].format;
		hr = m_device->CreateUnorderedAccessView(m_bloomBuffers[FirstPingPong].rtt, &uavDesc, m_bloomMipUAVs[FirstPingPong][i].GetAddressOf());
		assert(SUCCEEDED(hr) && "Error, unordered access view could not be created!");

		texName = L"FirstPingPingBloomBuffer " + std::to_wstring(i) + L" UAV";
		m_bloomMipUAVs[FirstPingPong][i]->SetPrivateData(WKPDID_D3DDebugObjectNameW, (UINT)sizeof(texName), texName.c_str());

		// - Second Ping Pong Buffer
		uavDesc.Format = m_bloomBuffers[SecondPingPong].format;
		hr = m_device->CreateUnorderedAccessView(m_bloomBuffers[SecondPingPong].rtt, &uavDesc, m_bloomMipUAVs[SecondPingPong][i].GetAddressOf());
		assert(SUCCEEDED(hr) && "Error, unordered access view could not be created!");

		texName = L"SecondPingPingBloomBuffer " + std::to_wstring(i) + L" UAV";
		m_bloomMipUAVs[SecondPingPong][i]->SetPrivateData(WKPDID_D3DDebugObjectNameW, (UINT)sizeof(texName), texName.c_str());
	}

	// Shaders
	ShaderFiles sf;
	sf.cs = L"BloomDownsamplingCS.hlsl";
	m_bloomDownsampleShader.initialize(m_device.Get(), m_deviceContext.Get(), sf);

	sf.cs = L"BloomUpsamplingCS.hlsl";
	m_bloomUpsampleShader.initialize(m_device.Get(), m_deviceContext.Get(), sf);

	// Constant Buffer
	m_bloomUpsampleData = std::make_unique< CS_UPSAMPLE_CBUFFER >();
	m_bloomUpsampleBuffer.initialize(m_device.Get(), m_deviceContext.Get(), m_bloomUpsampleData.get(), BufferType::CONSTANT);
	m_bloomDownsampleData = std::make_unique< CS_DOWNSAMPLE_CBUFFER >();
	m_bloomDownsampleData->threshold = XMFLOAT4(m_bloomThreshold, m_bloomThreshold - m_bloomKnee, m_bloomKnee * 2, 0.25f / m_bloomKnee);
	m_bloomDownsampleBuffer.initialize(m_device.Get(), m_deviceContext.Get(), m_bloomDownsampleData.get(), BufferType::CONSTANT);
}

void RenderHandler::initAdaptiveExposurePass()
{
	// Shaders
	ShaderFiles sf;
	sf.cs = L"AdaptiveExposureHistogramCS.hlsl";
	m_adaptiveExposureHistogramShaders.initialize(m_device.Get(), m_deviceContext.Get(), sf);

	sf.cs = L"AdaptiveExposureAveragingCS.hlsl";
	m_adaptiveExposureAveragingShaders.initialize(m_device.Get(), m_deviceContext.Get(), sf);

	// Constant Buffers
	m_histogramCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), &m_histogramCData, BufferType::CONSTANT);
	m_histogramAveragingCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), &m_histogramAveragingCData, BufferType::CONSTANT);

	// Buffers

	// - Histogram texture (16 x 16)
	UINT histogramWidth = 16;
	UINT histogramHeight = 16;
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(uint32_t) * histogramWidth * histogramHeight;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	bufferDesc.StructureByteStride = 0;
	//bufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	D3D11_SUBRESOURCE_DATA histogramData;
	histogramData.pSysMem;

	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &buffer);
	assert(SUCCEEDED(hr) && "Error, histogram buffer could not be created!");

	//  - Histogram UAV
	D3D11_BUFFER_UAV uavBufferDesc = {};
	uavBufferDesc.FirstElement = 0;
	uavBufferDesc.NumElements = histogramWidth * histogramHeight;
	uavBufferDesc.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer = uavBufferDesc;
	uavDesc.Texture2D.MipSlice = 0;
	
	hr = m_device->CreateUnorderedAccessView(buffer, &uavDesc, m_histogramUAV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, histogram unordered access view could not be created!");

	buffer->Release();

	// - Luminance texture (1 x 1)
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = 1;
	textureDesc.Height = 1;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = m_device->CreateTexture2D(&textureDesc, NULL, &texture);
	assert(SUCCEEDED(hr) && "Error, luminance texture could not be created!");

	// - Luminance SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = m_device->CreateShaderResourceView(texture, &srvDesc, m_luminanceSRV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, luminance shader resource view could not be created!");

	// - Luminance UAV
	uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	hr = m_device->CreateUnorderedAccessView(texture, &uavDesc, m_luminanceUAV.GetAddressOf());
	assert(SUCCEEDED(hr) && "Error, luminance unordered access view could not be created!");

	texture->Release();
}

void RenderHandler::calculateBlurWeights(CS_BLUR_CBUFFER* bufferData, int radius, float sigma)
{
	// One Dimensional weight calculation
	/*bufferData->direction = 0;
	bufferData->radius = radius;

	float sum = 0.f;
	float twoSigmaSq = 2 * sigma * sigma;

	for (size_t i = 0; i <= bufferData->radius; ++i)
	{
		float temp = (1.f / sigma) * std::expf(-static_cast<float>(i * i) / twoSigmaSq);
		bufferData->weights[i] = temp;
		sum += 2 * temp;
	}
	sum -= bufferData->weights[0];

	float normalizationFactor = 1.f / sum;
	for (int i = 0; i <= bufferData->radius; ++i)
		bufferData->weights[i] *= normalizationFactor;*/


	bufferData->direction = 0;
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);
	bufferData->radius = blurRadius;

	assert(blurRadius <= MAX_BLUR_RADIUS);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		bufferData->weights[i + blurRadius] = expf(-x * x / twoSigma2);
		weightSum += bufferData->weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	float weightLength = 2.f * (float)blurRadius + 1.f;
	for (int i = 0; i < weightLength; ++i)
		bufferData->weights[i] /= weightSum;
}

void RenderHandler::lightPass()
{
	// Set Output Render Target
	m_deviceContext->OMSetRenderTargets(1, &m_hdrRTV.rtv, nullptr);

	// Constant Buffer
	
	// - Camera Buffer
	m_deviceContext->PSSetConstantBuffers(0, 1, m_camera.getConstantBuffer());

	// - Lights Buffer
	m_deviceContext->PSSetConstantBuffers(1, 1, m_lightManager.GetAddressOf());

	// - Shadow Buffer, For Volumetric Sun Scattering
	m_shadowInstance.bindLightMatrixPS();

	// Set G-Buffer Shader Resource views
	ID3D11ShaderResourceView* gBufferSRVs[] =
	{
		m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC].srv,
		m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv,
		m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK].srv,
		m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].srv,
		m_gBuffer.renderTextures[GBufferType::DEPTH].srv,
		m_volumetricAccumulationRTV.srv
	};
	
	if (m_ssaoToggle)
	{
		if (m_useHBAOToggle)
			gBufferSRVs[GBufferType::AMBIENT_OCCLUSION] = m_HBAOInstance.getAORenderTexture().srv;
		else
			gBufferSRVs[GBufferType::AMBIENT_OCCLUSION] = m_SSAOInstance.getAORenderTexture().srv;
	}
	
	UINT srvIndex = GBufferType::GB_NUM + 1;

	m_deviceContext->PSSetShaderResources(0, srvIndex, gBufferSRVs);

	// Set Specular radiance and Diffuse irradiance maps
	srvIndex++;
	m_sky.setSkyTextures(srvIndex, srvIndex - 1);

	// Set Light Pass Shaders
	m_lightPassShaders.setShaders();

	// Draw Fullscreen Quad
	m_deviceContext->Draw(4, 0);

	// Unbind Shader Resource Views
	m_deviceContext->PSSetShaderResources(0, srvIndex, m_shaderResourcesNullptr);
}

void RenderHandler::downsamplePass()
{
	/*m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, NULL);
	m_downsampleCS.setShaders();

	m_deviceContext->CSSetShaderResources(0, 1, &m_SSAOInstance.getAORenderTexture().srv);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, m_downSampledUnorderedAccessView.GetAddressOf(), 0);
	m_deviceContext->Dispatch(m_settings.width / 16, m_settings.height / 16, 1);

	m_deviceContext->CSSetShaderResources(0, 1, &m_shaderResourceNullptr);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessNullptr, 0);*/
}

void RenderHandler::blurSSAOPass()
{
	UINT cOffset = -1;

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, NULL);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessNullptr, &cOffset);
	m_deviceContext->PSSetShaderResources(0, 2, m_shaderResourcesNullptr);

	m_edgePreservingBlurCS.setShaders();

	ID3D11ShaderResourceView* blurSRVs[] = { m_SSAOInstance.getAORenderTexture().srv, m_blurPingPongSRV.Get() };
	ID3D11UnorderedAccessView* blurUAVs[] = { m_blurPingPongUAV.Get(), m_SSAOInstance.getAORenderTexture().uav };

	for (UINT i = 0; i < 2; i++)
	{
		// Blur Constant Buffer
		m_blurCData->direction = i;
		
		m_blurCBuffer.update(m_blurCData.get());
		m_deviceContext->CSSetConstantBuffers(0, 1, m_blurCBuffer.GetAddressOf());

		// Set Rescources
		m_deviceContext->CSSetShaderResources(0, 1, &blurSRVs[m_blurCData->direction]);
		m_deviceContext->CSSetShaderResources(1, 1, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv);
		m_deviceContext->CSSetShaderResources(2, 1, &m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv);
		m_deviceContext->CSSetUnorderedAccessViews(0, 1, &blurUAVs[m_blurCData->direction], &cOffset);

		// Dispatch Shader
		UINT clientWidth = (UINT)std::ceil((m_clientWidth / 16.f) + 0.5f);
		UINT clientHeight = (UINT)std::ceil((m_clientHeight / 16.f) + 0.5f);
		m_deviceContext->Dispatch(clientWidth, clientHeight, 1);

		// Unbind Unordered Access View and Shader Resource View
		m_deviceContext->CSSetShaderResources(0, 1, &m_shaderResourceNullptr);
		m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessNullptr, &cOffset);
	}

	m_deviceContext->CSSetShaderResources(0, 3, m_shaderResourcesNullptr);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, NULL);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessNullptr, &cOffset);
}

void RenderHandler::volumetricSunPass()
{
	// Set Accumulation Render Target
	m_deviceContext->OMSetRenderTargets(1, &m_volumetricAccumulationRTV.rtv, nullptr);

	// Shader
	m_volumetricSunShaders.setShaders();

	// Constant Buffer
	
	// - Light Inverse Matrix
	m_shadowInstance.bindInverseVpMatrixVS();

	// - Camera Data
	m_deviceContext->PSSetConstantBuffers(0, 1, m_camera.getConstantBuffer());

	// Textures

	// - Depth Texture
	m_deviceContext->PSSetShaderResources(0, 1, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv);
	// - Shadow Map
	m_deviceContext->PSSetShaderResources(6, 1, m_shadowInstance.getShadowMapSRV());

	// Draw Fullscreen Quad
	m_deviceContext->Draw(4, 0);

	// Reset
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, nullptr);
}

void RenderHandler::bloomPass()
{
	UINT cOffset = -1;
	UINT mipLevel;
	float mipWidth;
	float mipHeight;
	UINT texWidth;
	UINT texHeight;
	
	// Downsample / Threshold Pass
	
	// - Unbind orignal Texture (HDR Texture)
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, NULL);

	// - Setup for First Mip
	m_deviceContext->CSSetShaderResources(0, 1, &m_hdrRTV.srv);

	if (m_adaptiveExposureToggle)
		m_deviceContext->CSSetShaderResources(1, 1, m_luminanceSRV.GetAddressOf());
	else
	{
		auto* greyTexture = ResourceHandler::getInstance().getTexture(L"DefaultGrey.jpg");
		m_deviceContext->CSSetShaderResources(1, 1, &greyTexture);
	}
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_bloomBuffers[Base].uav, &cOffset); // Only works for mip 0
	m_deviceContext->CSSetUnorderedAccessViews(1, 1, &m_bloomBuffers[FirstPingPong].uav, &cOffset); // Only works for mip 0
	m_bloomDownsampleShader.setShaders();

	// - Update Constant Buffer
	m_bloomDownsampleData->mipDimensions = XMFLOAT2(1.f / (float)m_clientWidth, 1.f / (float)m_clientHeight);
	m_bloomDownsampleData->mipLevel = 0.f;

	m_bloomDownsampleBuffer.update(m_bloomDownsampleData.get());
	m_deviceContext->CSSetConstantBuffers(0, 1, m_bloomDownsampleBuffer.GetAddressOf());

	// - Dispatch
	texWidth = (UINT)std::ceil((m_clientWidth / 16.f) + 0.5f);
	texHeight = (UINT)std::ceil((m_clientHeight / 16.f) + 0.5f);
	m_deviceContext->Dispatch(texWidth, texHeight, 1);
	 
	// - Unbind Shader Resource Views and Unordered Access View
	m_deviceContext->CSSetShaderResources(0, 1, m_shaderResourcesNullptr);
	m_deviceContext->CSSetUnorderedAccessViews(0, 2, m_unorderedAccessesNullptr, &cOffset);

	// - Loop through rest of Mip Chain
	bool flipper = true;
	for (size_t i = 0; i < NR_OF_BLOOM_MIPS - 1; ++i)
	{
		mipLevel = (UINT)i; // Input Mip Texture

		// - Send Ouput Texture, the Base Buffer(0) will contain all Final Downsampled Mips
		m_deviceContext->CSSetUnorderedAccessViews(0, 1, m_bloomMipUAVs[Base][mipLevel + 1].GetAddressOf(), &cOffset);

		// - Send Input Texture and Output Buffer Texture
		if (flipper)
		{
			m_deviceContext->CSSetShaderResources(0, 1, &m_bloomBuffers[FirstPingPong].srv);
			m_deviceContext->CSSetUnorderedAccessViews(1, 1, m_bloomMipUAVs[SecondPingPong][mipLevel + 1].GetAddressOf(), &cOffset);
		}
		else
		{
			m_deviceContext->CSSetShaderResources(0, 1, &m_bloomBuffers[SecondPingPong].srv);
			m_deviceContext->CSSetUnorderedAccessViews(1, 1, m_bloomMipUAVs[FirstPingPong][mipLevel + 1].GetAddressOf(), &cOffset);
		}
		flipper = !flipper;

		// - Update Constant Buffer
		mipWidth = (float)((m_clientWidth / 2) >> mipLevel); // Get width of output texture
		mipHeight = (float)((m_clientHeight / 2) >> mipLevel);

		m_bloomDownsampleData->mipDimensions = XMFLOAT2(1.f / mipWidth, 1.f / mipHeight);
		m_bloomDownsampleData->mipLevel = (float)mipLevel;

		m_bloomDownsampleBuffer.update(m_bloomDownsampleData.get());
		m_deviceContext->CSSetConstantBuffers(0, 1, m_bloomDownsampleBuffer.GetAddressOf());

		// - Dispatch
		texWidth = (UINT)std::ceil((mipWidth / 16.f) + 0.5f);
		texHeight = (UINT)std::ceil((mipHeight / 16.f) + 0.5f);
		m_deviceContext->Dispatch(texWidth, texHeight, 1);

		// - Unbind Shader Resource Views and Unordered Access View
		m_deviceContext->CSSetShaderResources(0, 1, m_shaderResourcesNullptr);
		m_deviceContext->CSSetUnorderedAccessViews(0, 2, m_unorderedAccessesNullptr, &cOffset);
	}

	// Upsample
	
	// - Setup
	m_bloomUpsampleShader.setShaders();
	m_deviceContext->CSSetShaderResources(0, 1, &m_bloomBuffers[Base].srv);

	// - Loop through Mip Chain
	flipper = true;
	for (size_t i = NR_OF_BLOOM_MIPS; i > 0; --i)
	{
		mipLevel = (UINT)i - 1;

		// - Send Ouput Texture and Last Result Texture, ignored by shader in the first pass
		if (flipper)
		{
			m_deviceContext->CSSetShaderResources(1, 1, &m_bloomBuffers[FirstPingPong].srv);
			m_deviceContext->CSSetUnorderedAccessViews(0, 1, m_bloomMipUAVs[SecondPingPong][mipLevel].GetAddressOf(), &cOffset);
		}
		else
		{
			m_deviceContext->CSSetShaderResources(1, 1, &m_bloomBuffers[SecondPingPong].srv);
			m_deviceContext->CSSetUnorderedAccessViews(0, 1, m_bloomMipUAVs[FirstPingPong][mipLevel].GetAddressOf(), &cOffset);
		}
		flipper = !flipper;

		// - Update Constant Buffer
		mipWidth = (float)(m_clientWidth >> mipLevel);
		mipHeight = (float)(m_clientHeight >> mipLevel);

		m_bloomUpsampleData->mipDimensions = XMFLOAT2(1.f / mipWidth, 1.f / mipHeight);
		m_bloomUpsampleData->mipLevel = (float)mipLevel;

		m_bloomUpsampleBuffer.update(m_bloomUpsampleData.get());
		m_deviceContext->CSSetConstantBuffers(0, 1, m_bloomUpsampleBuffer.GetAddressOf());

		// - Dispatch
		texWidth = (UINT)std::ceil((mipWidth / 16.f) + 0.5f);
		texHeight = (UINT)std::ceil((mipHeight / 16.f) + 0.5f);
		m_deviceContext->Dispatch(texWidth, texHeight, 1);

		// - Unbind Shader Resource Views and Unordered Access View
		m_deviceContext->CSSetShaderResources(1, 1, &m_shaderResourceNullptr);
		m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessNullptr, &cOffset);
	}
}

void RenderHandler::adaptiveExposurePass(float deltaTime)
{
	UINT cOffset = -1;

	// - Unbind orignal Texture (HDR Texture)
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, NULL);

	// Histogram Generation
	m_deviceContext->CSSetShaderResources(0, 1, &m_hdrRTV.srv);
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, m_histogramUAV.GetAddressOf(), &cOffset);
	m_adaptiveExposureHistogramShaders.setShaders();

	m_histogramCData.texWidth = m_clientWidth;
	m_histogramCData.texHeight = m_clientHeight;
	m_histogramCBuffer.update(&m_histogramCData);
	m_deviceContext->CSSetConstantBuffers(0, 1, m_histogramCBuffer.GetAddressOf());

	// - Dispatch
	m_deviceContext->Dispatch(m_clientWidth / 16, m_clientHeight / 16, 1);

	// Luminance Calculation

	// m_histogramUAV already bound to slot 0
	m_deviceContext->CSSetUnorderedAccessViews(1, 1, m_luminanceUAV.GetAddressOf(), &cOffset);
	m_adaptiveExposureAveragingShaders.setShaders();

	m_histogramAveragingCData.pixelCount = m_clientWidth * m_clientHeight;
	m_histogramAveragingCData.deltaTime = deltaTime;
	m_histogramAveragingCBuffer.update(&m_histogramAveragingCData);
	m_deviceContext->CSSetConstantBuffers(0, 1, m_histogramAveragingCBuffer.GetAddressOf());

	// - Dispatch
	m_deviceContext->Dispatch(16, 16, 1);

	// Unbind UAVs
	m_deviceContext->CSSetUnorderedAccessViews(0, 2, m_unorderedAccessesNullptr, &cOffset);
}

void RenderHandler::particlePass()
{
	// Setup
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	UINT sampleMask = 0xffffffff;
	m_deviceContext->OMSetBlendState(m_blendStatePreMultipliedAlphaBlend.Get(), blendFactor, sampleMask);
	m_deviceContext->RSSetState(m_cullOffRasterizerState.Get());
	m_deviceContext->PSSetShaderResources(1, 1,&m_shaderResourceNullptr); // Unbind Normal Texture

	for (auto& object : m_particleSystems)
	{
		m_deviceContext->OMSetDepthStencilState(m_disabledDepthStencilState.Get(), 0);

		m_deviceContext->OMSetRenderTargets(1, &m_hdrRTV.rtv, nullptr); // Unbind Depth Buffer
		m_deviceContext->GSSetShaderResources(1, 1, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv); // Used for Particle Collision
		m_deviceContext->GSSetShaderResources(2, 1, &m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv); // Used for Particle Collision

		// Generate
		object.second.generateParticles();

		// Render Setup
		m_deviceContext->OMSetDepthStencilState(m_readOnlyDepthStencilState.Get(), 0);

		m_deviceContext->GSSetShaderResources(1, 2, m_shaderResourcesNullptr); // Unbind G-Buffer textures
		m_deviceContext->OMSetRenderTargets(1, &m_hdrRTV.rtv, m_depthStencilView.Get()); // Bind Depth Buffer Back

		// Render
		object.second.renderParticles();
	}

	// Reset
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
	m_deviceContext->OMSetRenderTargets(1, &m_hdrRTV.rtv, m_depthStencilView.Get());
	m_deviceContext->RSSetState(m_defaultRasterizerState.Get());
	m_deviceContext->OMSetBlendState(m_blendStateBlend.Get(), blendFactor, sampleMask);
}

void RenderHandler::initCamera()
{
	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions

	m_camera.initialize(m_device.Get(), m_deviceContext.Get(), m_settings->fov, (float)winRect.right / (float)winRect.bottom, 0.1f, 1000.f);
}

void RenderHandler::initialize(HWND* window, Settings* settings)
{
	m_window = window;
	m_settings = settings;

	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions

	initDeviceAndSwapChain();
	initRenderTargets();
	initViewPort();
	initDepthStencilBuffer();
	initRenderStates();
	initCamera();
	ImGui_ImplDX11_Init(m_device.Get(), m_deviceContext.Get());
	ResourceHandler::getInstance().initialize(m_device.Get(), m_deviceContext.Get());

	// Lighting
	m_lightManager.initialize(m_device.Get(), m_deviceContext.Get(), m_camera.getViewMatrixPtr(), m_camera.getProjectionMatrixPtr());
	m_shadowInstance.initialize(m_device.Get(), m_deviceContext.Get(), SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	
	// Sky
	Light sunLight;
	sunLight.direction = XMFLOAT3(0, -1, 0);
	sunLight.color = XMFLOAT3(0.8f, 0.6f, 0.5f);
	sunLight.intensity = 1.f;
	sunLight.isCastingShadow = true;
	Light moonLight;
	moonLight.direction = XMFLOAT3(0, -1, 0);
	moonLight.color = XMFLOAT3(0.2f, 0.3f, 0.4f);
	moonLight.intensity = 0.5f;
	moonLight.isCastingShadow = true;
	//m_sky.initialize(m_device.Get(), m_deviceContext.Get(), L"TableMountain1Cubemap.dds", L"TableMountain1Irradiance.dds");
	m_sky.initialize(m_device.Get(), m_deviceContext.Get(), &m_shadowInstance, sunLight, moonLight, L"dikhololo_night_skybox.dds", L"dikhololo_night_sky_irradiance.dds");
	
	// - Render Cubemap Previews
	m_deviceContext->RSSetState(m_defaultRasterizerState.Get());
	m_sky.cubemapPreviewsRenderSetup();
	m_deviceContext->Draw(4, 0);

	// Timer
	m_timer.restart();

	// Shader States
	m_shaderStates.resize(ShaderStates::NUM);
	ShaderFiles shaderFiles;
	// - Phong
	shaderFiles.vs = L"GeneralVS.hlsl";
	shaderFiles.ps = L"GBufferPS.hlsl";
	m_shaderStates[ShaderStates::PHONG].initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);
	// - PBR
	shaderFiles.vs = L"GeneralVS.hlsl";
	shaderFiles.ps = L"GBufferPBR_PS.hlsl";
	m_shaderStates[ShaderStates::PBR].initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);

	// - Light pass Shaders
	shaderFiles.vs = L"FullscreenQuadVS.hlsl";
	shaderFiles.ps = L"LightPassPS.hlsl";
	m_lightPassShaders.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS);

	// SSAO
	m_HBAOInstance.initialize(m_device.Get(), m_deviceContext.Get(), winRect.right, winRect.bottom, m_camera.getFarZ(), m_camera.getFov(), m_camera.getViewMatrix(), m_camera.getProjectionMatrix());
	m_SSAOInstance.initialize(m_device.Get(), m_deviceContext.Get(), winRect.right, winRect.bottom, m_camera.getFarZ(), m_camera.getFov(), m_camera.getViewMatrix(), m_camera.getProjectionMatrix());

	// Blur
	initSSAOBlurPass(m_clientWidth, m_clientHeight, m_SSAOInstance.getAORenderTexture().format);
	
	// Volumetric Sun Scattering
	initVolumetricSunPass();

	// Bloom
	initBloomPass((UINT)winRect.right, (UINT)winRect.bottom);

	// Lens Flare
	m_lensFlareCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), nullptr, BufferType::CONSTANT);

	shaderFiles.vs = L"LensFlareVS.hlsl";
	shaderFiles.gs = L"LensFlareGS.hlsl";
	shaderFiles.ps = L"LensFlarePS.hlsl";
	m_lensFlareShaders.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS, D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_lensFlareTexturesSRV = ResourceHandler::getInstance().getTexture(L"Lens flare/lens_flare_textures.dds");

	// Particles

	// - Fire
	PARTICLE_STYLE particleStyle;
	particleStyle.colorBegin = XMFLOAT3(1.f, .3f, .16f);
	particleStyle.colorBias = 0.5f;
	particleStyle.colorEnd = XMFLOAT3(0.f, 0.f, 0.f);
	particleStyle.colorIntensity = 1.f;
	particleStyle.scaleVariationMax = 0.f;
	particleStyle.rotationVariationMax = 0.f;
	particleStyle.lifetime = 1.f;
	particleStyle.useNoise = true;
	particleStyle.emitDirection = XMFLOAT3(0.f, 15.f, 0.f);
	particleStyle.emitInterval = 0.1f;
	particleStyle.randomizePosition = false;
	particleStyle.randomizeDirection = false;
	particleStyle.dieOnCollition = true;
	particleStyle.fadeInAndOut = false;
	particleStyle.idInterval = 5;

	for (int i = 0; i < 4; i++)
		m_particleSystems["fire" + std::to_string(i)].Initialize(m_device.Get(), m_deviceContext.Get(), L"spot_gradient_tex.png", 20, particleStyle, XMFLOAT3(0,0,0), XMFLOAT2(3.f, 3.f));
	
	m_particleSystems["fire0"].setEmitPosition(XMFLOAT3(-8.85f, 4.8f, -23.7f));
	m_particleSystems["fire1"].setEmitPosition(XMFLOAT3( 8.2f, 4.8f, -23.7f));
	m_particleSystems["fire2"].setEmitPosition(XMFLOAT3(-8.85f, 4.8f,  22.8f));
	m_particleSystems["fire3"].setEmitPosition(XMFLOAT3( 8.2f, 4.8f,  22.8f));

	// Air
	particleStyle.colorBegin = XMFLOAT3(1.f, 1.f, 1.f);
	particleStyle.colorBias = 0.f;
	particleStyle.colorEnd = XMFLOAT3(0.5f, 0.5f, 0.5f);
	particleStyle.colorIntensity = 1.f;
	particleStyle.scaleVariationMax = 1.2f;
	particleStyle.rotationVariationMax = 0.f;
	particleStyle.lifetime = 4.f;
	particleStyle.useNoise = false;
	particleStyle.emitDirection = XMFLOAT3(0.f, 5.f, 0.f);
	particleStyle.emitInterval = 0.01f;
	particleStyle.randomizePosition = true;
	particleStyle.randomizePosBounds = XMFLOAT3(20.f, 15.f, 40.f);
	particleStyle.randomizeDirection = true;
	particleStyle.dieOnCollition = false;
	particleStyle.fadeInAndOut = true;
	particleStyle.idInterval = 300;

	m_particleSystems["air"].Initialize(m_device.Get(), m_deviceContext.Get(), L"spot_gradient_tex.png", 400, particleStyle, XMFLOAT3(0, 0, 0), XMFLOAT2(0.1f, 0.1f));
	m_particleSystems["air"].setEmitPosition(XMFLOAT3(0.f, 10.f, 0.f));

	// Selection
	
	// - Handler
	m_modelSelectionHandler.initialize(m_device.Get(), m_deviceContext.Get(), m_camera.getViewMatrixPtr(), m_camera.getProjectionMatrixPtr());
	
	// - Shaders
	shaderFiles.vs = L"SelectionVS.hlsl";
	shaderFiles.ps = L"SelectionPS.hlsl";
	m_selectionShaders.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);
	
	// - Buffer
	m_selectionAnimationData.colorOpacity = 0.f;
	PS_COLOR_ANIMATION_BUFFER* selectionAnimationData = new PS_COLOR_ANIMATION_BUFFER(m_selectionAnimationData);
	m_selectionCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), selectionAnimationData, BufferType::CONSTANT);

	// Adaptive Exposure
	initAdaptiveExposurePass();

	// HDR Tonemapping
	shaderFiles.vs = L"FullscreenQuadVS.hlsl";
	shaderFiles.gs = L"";
	shaderFiles.ps = L"HDRtoSDR_PS.hlsl";
	m_tonemapShaders.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS);
	m_tonemapCBuffer.initialize(m_device.Get(), m_deviceContext.Get(), &m_tonemapCData, BufferType::CONSTANT);
}

UINT RenderHandler::getClientWidth() const
{
	return m_clientWidth;
}

UINT RenderHandler::getClientHeight() const
{
	return m_clientHeight;
}

void RenderHandler::updateCamera(XMVECTOR position, XMVECTOR rotation)
{
	m_camera.updateViewMatrix(position, rotation);
	m_sky.updateMatrices(m_camera.getViewMatrix(), m_camera.getProjectionMatrix(), m_camera.getCameraPosition());

	if (m_useHBAOToggle)
		m_HBAOInstance.updateViewMatrix(m_camera.getViewMatrix());
	else
		m_SSAOInstance.updateViewMatrix(m_camera.getViewMatrix());

	//m_shadowInstance.buildLightMatrix(m_camera.getCameraPositionF3());
}

RenderObjectKey RenderHandler::newRenderObject(std::string modelName, ShaderStates shaderState)
{
	RenderObjectList* objects = nullptr;
	switch (shaderState)
	{
	case PHONG:
		objects = &m_renderObjects;
		break;
	case PBR:
		objects = &m_renderObjectsPBR;
		break;
	default:
		break;
	}

	RenderObjectKey key;
	key.key = (int)objects->size();
	key.valid = true;
	key.objectType = shaderState;

	(*objects)[key] = new RenderObject();
	objects->at(key)->initialize(m_device.Get(), m_deviceContext.Get(), (int)objects->size(), modelName);
	objects->at(key)->setShaderState(shaderState);
	if (m_camera.isInitialized())
	{
		XMMATRIX viewMatrix = m_camera.getViewMatrix();
		XMMATRIX projMatrix = m_camera.getProjectionMatrix();
		objects->at(key)->updateWCPBuffer(XMMatrixIdentity(), viewMatrix, projMatrix);
	}

	return key;
}

void RenderHandler::setRenderObjectTextures(RenderObjectKey key, TexturePaths textures)
{
	switch (key.objectType)
	{
	case PHONG:
		m_renderObjects[key]->setTextures(textures);
		break;
	default:
		break;
	}
}

void RenderHandler::setRenderObjectTextures(RenderObjectKey key, TexturePathsPBR textures)
{
	switch (key.objectType)
	{
	case PBR:
		m_renderObjectsPBR[key]->setTextures(textures);
		break;
	default:
		break;
	}
}

void RenderHandler::setRenderObjectMaterial(RenderObjectKey key, PS_MATERIAL_BUFFER material)
{
	switch (key.objectType)
	{
	case ShaderStates::PHONG:
		m_renderObjects[key]->setMaterial(material);
		break;
	default:
		break;
	}
}

void RenderHandler::setRenderObjectMaterialPBR(RenderObjectKey key, PS_MATERIAL_PBR_BUFFER material)
{
	switch (key.objectType)
	{
	case ShaderStates::PBR:
		m_renderObjectsPBR[key]->setMaterial(material);
		break;
	default:
		break;
	}
}

void RenderHandler::updateRenderObjectWorld(RenderObjectKey key, XMMATRIX worldMatrix)
{
	XMMATRIX viewMatrix = m_camera.getViewMatrix();
	XMMATRIX projMatrix = m_camera.getProjectionMatrix();
	switch (key.objectType)
	{
	case PHONG:
		m_renderObjects[key]->updateWCPBuffer(worldMatrix, viewMatrix, projMatrix);
		break;
	case PBR:
		m_renderObjectsPBR[key]->updateWCPBuffer(worldMatrix, viewMatrix, projMatrix);
		break;
	default:
		break;
	}
}

void RenderHandler::deleteRenderObject(RenderObjectKey key)
{
	switch (key.objectType)
	{
	case PHONG:
		m_renderObjects.erase(key);
		break;
	case PBR:
		m_renderObjectsPBR.erase(key);
		break;
	default:
		break;
	}
	OutputDebugString(L"RenderObject Removed! \n");
}

RenderObjectKey RenderHandler::setShaderState(RenderObjectKey key, ShaderStates shaderState)
{
	switch (key.objectType)
	{
	case PHONG:
		if (shaderState != key.objectType)
		{
			m_renderObjects[key]->setShaderState(shaderState);
			RenderObjectKey newKey;
			switch (shaderState)
			{
			case PBR:
				newKey.key = (int)m_renderObjectsPBR.size();
				newKey.valid = true;
				newKey.objectType = shaderState;
				m_renderObjectsPBR[newKey] = m_renderObjects[key];

				break;
			default:
				break;
			}
			m_renderObjects.erase(key);
			return newKey;
		}
		break;
	case PBR:
		if (shaderState != key.objectType)
		{
			m_renderObjectsPBR[key]->setShaderState(shaderState);
			RenderObjectKey newKey;
			switch (shaderState)
			{
			case PHONG:
				newKey.key = (int)m_renderObjects.size();
				newKey.valid = true;
				newKey.objectType = shaderState;
				m_renderObjects[newKey] = m_renderObjectsPBR[key];

				break;
			default:
				break;
			}
			m_renderObjectsPBR.erase(key);
			return newKey;
		}
		break;
	default:
		break;
	}
	return key;
}

void RenderHandler::modelTextureUIUpdate(RenderObjectKey key)
{
	if (key.isValid())
	{
		switch (key.objectType)
		{
		case PHONG:
			m_renderObjects[key]->materialUIUpdate();
			break;
		case PBR:
			m_renderObjectsPBR[key]->materialUIUpdate();
			break;
		default:
			break;
		}
	}
}

int RenderHandler::addLight(Light newLight, XMFLOAT3 rotationRad, bool usedForShadowMapping)
{
	if (m_lightManager.addLight(newLight))
	{
		m_lightManager.update();
		//if (usedForShadowMapping && newLight.type == DIRECTIONAL_LIGHT)
		//{
		//	m_shadowInstance.buildLightMatrix(newLight, rotationRad);
		//	//m_shadowInstance.buildLightMatrix(newLight, rotationRad, m_camera.getCameraPositionF3());
		//}

		return m_lightManager.getNrOfLights(); // Used as a ID, SHOULD BE CHANGED TO A BETTER INDEX SYSTEM!!!
	}
	return -1;
}

void RenderHandler::removeLight(int id)
{
	m_lightManager.removeLight(id);
}

void RenderHandler::updateLight(Light* light, int id)
{
	m_lightManager.updateLight(light, id);

	/*if (light->isCastingShadow)
		m_shadowInstance.updateLight(*light);*/
}

void RenderHandler::changeShadowMappingLight(Light* light, XMFLOAT3 rotationRad, bool disableShadowCasting)
{
	if (disableShadowCasting)
	{
		m_shadowMappingEnabled = false;
		m_shadowInstance.clearShadowMap();
	}
	else
	{
		m_shadowMappingEnabled = true;
		XMVECTOR normDirection = XMLoadFloat3(&light->direction);
		XMVector4Normalize(normDirection);
		XMStoreFloat3(&light->direction, normDirection);
		m_shadowInstance.buildLightMatrix(*light, rotationRad/*, m_camera.getCameraPositionF3()*/);
	}
}

bool* RenderHandler::getWireframeModePtr()
{
	return &m_wireframeMode;
}

bool* RenderHandler::getSsaoModePtr()
{
	return &m_ssaoToggle;
}

bool* RenderHandler::getSsaoBlurModePtr()
{
	return &m_ssaoBlurToggle;
}

void RenderHandler::updateSelectedObject(RenderObjectKey key, XMFLOAT3 newPosition)
{
	m_selectedObjectKey = key;
	m_modelSelectionHandler.objectSelected(newPosition);
}

void RenderHandler::deselectObject()
{
	m_selectedObjectKey = RenderObjectKey(); // Invalid key
	m_modelSelectionHandler.objectDeselected();
}

XMFLOAT3 RenderHandler::getRayWorldDirection(UINT pointX, UINT pointY)
{
	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions

	XMMATRIX view = m_camera.getViewMatrix();
	XMMATRIX proj = m_camera.getProjectionMatrix();
	float fov = m_camera.getFov();
	float aspectRatio = (float)winRect.right / (float)winRect.bottom;

	// Projection conversion
	float projectionX = 1 / (aspectRatio * tan(fov / 2));
	float projectionY = 1 / tan(fov / 2);
	float viewX = (2.f * pointX / winRect.right - 1.f) / projectionX;
	float viewY = (-2.f * pointY / winRect.bottom + 1.f) / projectionY;
	float viewZ = 1.f;

	XMVECTOR rayDirection = XMVectorSet(viewX, viewY, viewZ, 1.f);

	// View Matrix
	XMVECTOR det = XMMatrixDeterminant(view);
	XMMATRIX viewInverse = XMMatrixInverse(&det, view);
	rayDirection = XMVector3Normalize(XMVector3TransformNormal(rayDirection, viewInverse));

	XMFLOAT3 rayDirectionF3;
	XMStoreFloat3(&rayDirectionF3, rayDirection);
	return rayDirectionF3;
}

float RenderHandler::selectionArrowPicking(UINT pointX, UINT pointY, char dimension)
{
	RECT winRect;
	GetClientRect(*m_window, &winRect); // Contains Client Dimensions

	XMMATRIX view = m_camera.getViewMatrix();
	XMMATRIX proj = m_camera.getProjectionMatrix();
	float fov = m_camera.getFov();
	float aspectRatio = (float)winRect.right / (float)winRect.bottom;
	
	// Projection conversion
	float projectionX = 1 / (aspectRatio * tan(fov / 2));
	float projectionY = 1 / tan(fov / 2);
	float viewX = (2.f * pointX / winRect.right - 1.f) / projectionX;
	float viewY = (-2.f * pointY / winRect.bottom + 1.f) / projectionY;
	float viewZ = 1.f;

	XMVECTOR rayOrigin = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	XMVECTOR rayDirection = XMVectorSet(viewX, viewY, viewZ, 1.f);

	// View Matrix
	XMVECTOR det = XMMatrixDeterminant(view);
	XMMATRIX viewInverse = XMMatrixInverse(&det, view);
	// World Matrix
	XMMATRIX world = m_modelSelectionHandler.getWorldMatrix(dimension);
	det = XMMatrixDeterminant(world);
	XMMATRIX worldInverse = XMMatrixInverse(&det, world);

	// Local conversion Matrix
	XMMATRIX localMatrix = XMMatrixMultiply(viewInverse, worldInverse);

	// Local conversion
	rayOrigin = XMVector3TransformCoord(rayOrigin, localMatrix);
	rayDirection = XMVector3Normalize(XMVector3TransformNormal(rayDirection, localMatrix));

	return m_modelSelectionHandler.picking(rayOrigin, rayDirection, dimension);
}

void RenderHandler::update(double dt)
{
	// Sky
	m_sky.update(dt);

	// Lens Flare
	if (m_lensFlareToggle)
	{
		XMFLOAT3 sunPos = m_sky.getSkyLightDirection(SkyLightType::SUN);
		XMVECTOR lightPos = XMLoadFloat3(&sunPos);
		lightPos = XMVectorNegate(DirectX::XMVectorScale(lightPos, 1000.f)) + m_camera.getCameraPosition();
		lightPos = XMVectorSetW(lightPos, 0.f);

		m_lensFlareCData.skyLightPosition = DirectX::XMVector3TransformCoord(lightPos, m_camera.getViewMatrix());
		if (DirectX::XMVectorGetZ(m_lensFlareCData.skyLightPosition) < 0.f) // Behind Camera
			m_lensFlareCData.skyLightPosition = XMVectorSet(0,0,-1,1);
		else
			m_lensFlareCData.skyLightPosition = DirectX::XMVector3TransformCoord(m_lensFlareCData.skyLightPosition, m_camera.getProjectionMatrix());

		m_lensFlareCData.screenDimensions = XMFLOAT2((float)m_clientWidth, (float)m_clientHeight);
		m_lensFlareCBuffer.update(&m_lensFlareCData);
		m_deviceContext->GSSetConstantBuffers(2, 1, m_lensFlareCBuffer.GetAddressOf());
	}
	
	// Particles
	for (auto& object : m_particleSystems)
		object.second.update(dt, (float)m_timer.timeElapsed(), m_camera);

	// Selection
	if (m_selectedObjectKey.valid)
	{
		m_selectionAnimationData.colorOpacity += (m_animationDirection * (float)dt);
		if (m_selectionAnimationData.colorOpacity >= .9f)
		{
			m_animationDirection = -1.f;
			m_selectionAnimationData.colorOpacity = .9f;
		}
		else if (m_selectionAnimationData.colorOpacity <= 0.2f)
		{
			m_animationDirection = 1.f;
			m_selectionAnimationData.colorOpacity = 0.2f;
		}

		PS_COLOR_ANIMATION_BUFFER* selectionAnimationData = new PS_COLOR_ANIMATION_BUFFER(m_selectionAnimationData);
		m_selectionCBuffer.update(&selectionAnimationData);
	}
}

void RenderHandler::resetParticles()
{
	for (auto& object : m_particleSystems)
		object.second.reset();
}

void RenderHandler::updateShaderState(ShaderStates shaderState)
{
	m_shaderStates[shaderState].updateShaders();
}

void RenderHandler::updatePassShaders()
{
	m_lightPassShaders.updateShaders();
	m_tonemapShaders.updateShaders();

	/*for (auto& object : m_particleSystems)
		object.second.updateShaders();*/

	if (m_useHBAOToggle)
		m_HBAOInstance.updateShaders();
	else
		m_SSAOInstance.updateShaders();
	//m_edgePreservingBlurCS.updateShaders();

	/*m_bloomDownsampleShader.updateShaders();
	m_bloomUpsampleShader.updateShaders();*/

	//m_volumetricSunShaders.updateShaders();
	
	m_sky.updateProceduralShaders();
	m_lensFlareShaders.updateShaders();

	/*m_sky.updatePreviewShaders();
	m_sky.cubemapPreviewsRenderSetup();
	m_deviceContext->Draw(4, 0);*/
}

void RenderHandler::UIRenderShadowMap()
{
	ImGui::Begin("Shadow Map");
	ImGui::Image(m_shadowInstance.getShadowMapSRVNoneConst(), ImVec2(400.f, 400.f));
	ImGui::End();
}

void RenderHandler::UIRenderPipelineTexturesWindow()
{
	ImGui::Begin("Render Pipeline Textures");
	
	ImGui::Image(m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	ImGui::Image(m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	ImGui::Image(m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	//ImGui::Image(m_gBuffer.renderTextures[GBufferType::DEPTH].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	
	// SSAO / HBAO
	if (m_ssaoToggle)
	{
		if (m_useHBAOToggle)
			ImGui::Image(m_HBAOInstance.getAORenderTexture().srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		else
			ImGui::Image(m_SSAOInstance.getAORenderTexture().srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		
		/*m_HBAOInstance.UIRenderDitherTextureWindow();
		m_SSAOInstance.updateUI();*/
	}
	else
		ImGui::Image(m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	
	// Bloom
	ImGui::Image(m_bloomBuffers[Base].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	if (NR_OF_BLOOM_MIPS % 2 == 0)
		ImGui::Image(m_bloomBuffers[FirstPingPong].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	else
		ImGui::Image(m_bloomBuffers[SecondPingPong].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	
	// Volumetric Sun Accumulation
	ImGui::Image(m_volumetricAccumulationRTV.srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	
	ImGui::End();
}

float ACESFilmicTonemapping(float x, float A, float B, float C, float D, float E, float F)
{
	return (((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - (E / F));
}

float ACESFilmicTonemappingPlot(void* dataPtr, int index)
{
	PS_TONEMAP_CBUFFER* data = (PS_TONEMAP_CBUFFER*)dataPtr;
	float HDR = index / (float)256 * 12.0f;
	return ACESFilmicTonemapping(HDR, data->ACESss, data->ACESls, data->ACESla, data->ACESts, data->ACEStn, data->ACEStd) /
		ACESFilmicTonemapping(data->LinearWhite, data->ACESss, data->ACESls, data->ACESla, data->ACESts, data->ACEStn, data->ACEStd);
}

void RenderHandler::UITonemappingWindow()
{
	ImGui::Begin("Tonemapping");

	ImGui::Checkbox("Adaptive Exposure", &m_adaptiveExposureToggle);
	if (m_adaptiveExposureToggle)
	{
		if (ImGui::CollapsingHeader("Adaptive Exposure", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(16.0f);
			{
				if (ImGui::DragFloat("Min Log Luminance", &m_histogramCData.minLogLuminance))
				{
					m_histogramAveragingCData.minLogLuminance = m_histogramCData.minLogLuminance;
					m_histogramCData.oneOverLogLuminanceRange = 1.f / (std::abs(m_histogramCData.minLogLuminance) + m_histogramAveragingCData.logLuminanceRange);
				}
		
				if (ImGui::DragFloat("Log Luminance Range", &m_histogramAveragingCData.logLuminanceRange))
					m_histogramCData.oneOverLogLuminanceRange = 1.f / (std::abs(m_histogramCData.minLogLuminance) + m_histogramAveragingCData.logLuminanceRange);
		
				ImGui::DragFloat("Tau", &m_histogramAveragingCData.tau);
		
				ImGui::Image(m_luminanceSRV.Get(), ImVec2(100.f, 100.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
			}
			ImGui::Unindent(16.0f);
		}
	}

	if (ImGui::DragFloat("Exposure", &m_tonemapCData.Exposure, 0.01f, -10.0f, 10.0f))
		m_bloomDownsampleData->exposure = m_tonemapCData.Exposure;
	
	ImGui::DragFloat("Gamma", &m_tonemapCData.Gamma, 0.01f, 0.01f, 5.0f);

	ImGui::PlotLines("ACES Filmic Tonemapping", &ACESFilmicTonemappingPlot, &m_tonemapCData, 256, 0, nullptr, 0.0f, 1.0f, ImVec2(0, 250));
	ImGui::DragFloat("Shoulder Strength", &m_tonemapCData.ACESss, 0.01f, 0.01f, 5.0f);
	ImGui::DragFloat("Linear Strength", &m_tonemapCData.ACESls, 0.01f, 0.0f, 100.0f);
	ImGui::DragFloat("Linear Angle", &m_tonemapCData.ACESla, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Toe Strength", &m_tonemapCData.ACESts, 0.01f, 0.01f, 1.0f);
	ImGui::DragFloat("Toe Numerator", &m_tonemapCData.ACEStn, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Toe Denominator", &m_tonemapCData.ACEStd, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Linear White", &m_tonemapCData.LinearWhite, 0.01f, 1.0f, 120.0f);

	if (ImGui::Button("Reset"))
		m_tonemapCData = PS_TONEMAP_CBUFFER();

	m_tonemapCBuffer.update(&m_tonemapCData);

	ImGui::End();
}

void RenderHandler::UIssaoSettings()
{
	if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_DefaultOpen))
	{
		m_ssaoToggle = true;
		ImGui::Indent(16.0f);
		ImGui::PushItemWidth(-16.f);
		
		// HBAO or SSAO Toggle
		//ImGui::Text("Use HBAO");
		//ImGui::SameLine();
		//const char* str_id = "useHBAOToggle";
		//
		//ImVec4* colors = ImGui::GetStyle().Colors;
		//ImVec2 p = ImGui::GetCursorScreenPos();
		//ImDrawList* draw_list = ImGui::GetWindowDrawList();

		//float height = 15.f;
		//float width = height * 1.5f;
		//float radius = height * 0.5f;

		//ImGui::InvisibleButton(str_id, ImVec2(width, height));
		//if (ImGui::IsItemClicked())
		//	m_useHBAOToggle = !m_useHBAOToggle;
		//ImGuiContext& gg = *GImGui;
		//float ANIM_SPEED = 0.085f;

		//if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && gg.LastActiveIdTimer < ANIM_SPEED)
		//	float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);

		//if (ImGui::IsItemHovered())
		//	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(m_useHBAOToggle ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
		//else
		//	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(m_useHBAOToggle ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
		//
		//draw_list->AddCircleFilled(ImVec2(p.x + radius + (m_useHBAOToggle ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
		
		// SSAO Blur
		if (ImGui::CollapsingHeader("Blur", ImGuiTreeNodeFlags_DefaultOpen))
		{
			m_ssaoBlurToggle = true;
			ImGui::Indent(16.0f);
			ImGui::PushItemWidth(-40.f);
			if (ImGui::DragFloat("Sigma", &m_ssaoBlurSigma, 0.1f, 1.f, 5.f))
				calculateBlurWeights(m_blurCData.get(), MAX_BLUR_RADIUS, m_ssaoBlurSigma);

			ImGui::PopItemWidth();
			ImGui::Unindent(16.0f);
		}
		else
			m_ssaoBlurToggle = false;
		ImGui::PopItemWidth();
		ImGui::Unindent(16.0f);
	}
	else
		m_ssaoToggle = false;
}

void RenderHandler::UIadaptiveExposureSettings()
{
	ImGui::Checkbox("Adaptive Exposure", &m_adaptiveExposureToggle);
}

void RenderHandler::UIVolumetricSunSettings()
{
	if (ImGui::Checkbox("Volumetric Sun", &m_volumetricSunToggle))
		m_lightManager.setVolumetricSunScattering(m_volumetricSunToggle);

	if (ImGui::Checkbox("Fog", &m_fogToggle))
		m_lightManager.setFog(m_fogToggle);
	
	/*if (m_volumetricSunToggle)
	{
		ImGui::Indent(16.0f);
		ImGui::Checkbox("Wireframe Mesh", &m_volumetricSunWireframeToggle);
		ImGui::Unindent(16.0f);
	}*/
}

void RenderHandler::UIbloomSettings()
{
	if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
	{
		m_bloomToggle = true;
		ImGui::Indent(16.0f);

		ImGui::PushItemWidth(-68.f);
		if (ImGui::DragFloat("Threshold", &m_bloomThreshold, 0.1f, 0.f, 10.f))
			m_bloomDownsampleData->threshold = XMFLOAT4(m_bloomThreshold, m_bloomThreshold - m_bloomKnee, m_bloomKnee * 2, 0.25f / m_bloomKnee);
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(-95.f);
		if (ImGui::DragFloat("Knee Softness", &m_bloomKnee, 0.01f, 0.00001f, 10.f))
			m_bloomDownsampleData->threshold = XMFLOAT4(m_bloomThreshold, m_bloomThreshold - m_bloomKnee, m_bloomKnee * 2, 0.25f / m_bloomKnee);
		ImGui::PopItemWidth();
			
		ImGui::Unindent(16.0f);
	}
	else
		m_bloomToggle = false;
}

void RenderHandler::UILensFlareSettings()
{
	ImGui::Checkbox("Lens Flare", &m_lensFlareToggle);
}

void RenderHandler::UIEnviormentPanel()
{
	if (ImGui::CollapsingHeader("Enviorment Panel", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float indentSize = 16.f;
		ImGui::Indent(indentSize);

		ImGui::Text("Sky Light");
		m_sky.updateUI();

		ImGui::Separator();
		bool* procederualSkyToggle = m_sky.getProduralSkyTogglePtr();
		if (*procederualSkyToggle)
		{
			PROCEDURAL_SKY_CBUFFER* proceduralData = m_sky.getProduralSkyDataPtr();

			ImGui::Text("Sky");
			if (ImGui::ColorEdit3("Sky Color", &proceduralData->skyColor.x, ImGuiColorEditFlags_Float))
				m_sky.updateProceduralData();
			if (ImGui::DragFloat("Sky Exponent", &proceduralData->skyExponent, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();
			if (ImGui::ColorEdit3("Sky Night Color", &proceduralData->skyNightColor.x, ImGuiColorEditFlags_Float))
				m_sky.updateProceduralData();
			if (ImGui::DragFloat("Sky Night Exponent", &proceduralData->skyNightExponent, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();
			ImGui::Separator();

			ImGui::Text("Horizon");
			bool* useSunMoonCol = (bool*)&proceduralData->useSunMoonColorforHorizon;
			if (ImGui::Checkbox("Use Sun/Moon Color", useSunMoonCol))
				m_sky.updateProceduralData();

			if (!*useSunMoonCol)
			{
				if (ImGui::ColorEdit3("Horizon Color", &proceduralData->horizonColor.x, ImGuiColorEditFlags_Float))
					m_sky.updateProceduralData();
				if (ImGui::ColorEdit3("Horizon Night Color", &proceduralData->horizonNightColor.x, ImGuiColorEditFlags_Float))
					m_sky.updateProceduralData();
			}

			ImGui::Separator();
			
			ImGui::Text("Ground");
			if (ImGui::ColorEdit3("Ground Color", &proceduralData->groundColor.x, ImGuiColorEditFlags_Float))
				m_sky.updateProceduralData();
			if (ImGui::DragFloat("Ground Exponent", &proceduralData->groundExponent, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();
			if (ImGui::ColorEdit3("Ground Night Color", &proceduralData->groundNightColor.x, ImGuiColorEditFlags_Float))
				m_sky.updateProceduralData();
			if (ImGui::DragFloat("Ground Night Exponent", &proceduralData->groundNightExponent, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();
			ImGui::Separator();

			ImGui::Text("Sun");
			if (ImGui::DragFloat("Sun Strength", &proceduralData->sunExponent, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();

			if (ImGui::DragFloat("Sun Radius A", &proceduralData->sunRadiusA, 0.001f, 0.001f, 10.f))
				m_sky.updateProceduralData();

			if (ImGui::DragFloat("Sun Radius B", &proceduralData->sunRadiusB, 0.001f, 0.001f, 10.f))
				m_sky.updateProceduralData();
			ImGui::Separator();

			ImGui::Text("Moon");
			if (ImGui::DragFloat("Moon Exponent", &proceduralData->moonExponent, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();

			if (ImGui::DragFloat("Moon Radius A", &proceduralData->moonRadiusA, 0.001f, 0.001f, 10.f))
				m_sky.updateProceduralData();

			if (ImGui::DragFloat("Moon Radius B", &proceduralData->moonRadiusB, 0.001f, 0.001f, 10.f))
				m_sky.updateProceduralData();
			ImGui::Separator();

			ImGui::Text("Stars");
			if (ImGui::DragFloat("Stars Scale", &proceduralData->starsUVScale, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();

			if (ImGui::DragFloat("Stars Intensity", &proceduralData->starsIntensity, 0.1f, 0.1f, 10.f))
				m_sky.updateProceduralData();
			ImGui::Separator();
		}

		float imageSize = 100.f;
		float imageOffset = imageSize + indentSize + 10.f;
		_bstr_t nameCStr;

		m_sky.ambientSettingsUI();

		if (!*procederualSkyToggle)
		{
			ImGui::Text("Skybox Cubemap");
			nameCStr = m_sky.getSkyFileName().c_str();
			ImGui::Image(m_sky.getSkyPreviewSRV(), ImVec2(imageSize, imageSize));
			ImGui::SameLine(imageOffset);
		}
		ImGui::BeginGroup();
		{
			if (!*procederualSkyToggle)
			{
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_loadNewCubemapType = CubemapType::Skybox;
				}
			}
			m_lightManager.enviormentSpecContributionUI();
		}
		ImGui::EndGroup();
		
		if (!*procederualSkyToggle)
		{
			ImGui::Text("Irradiance Cubemap");
			nameCStr = m_sky.getIrradianceFileName().c_str();
			ImGui::Image(m_sky.getIrradiancePreviewSRV(), ImVec2(imageSize, imageSize));
			ImGui::SameLine(imageOffset);
		}
		ImGui::BeginGroup();
		{
			if (!*procederualSkyToggle)
			{
				if (ImGui::Button((const char*)nameCStr))
				{
					m_fileDialog.Open();
					m_fileDialog.SetPwd(std::filesystem::current_path() / "Textures");
					m_loadNewCubemapType = CubemapType::Irradiance;
				}
			}
			m_lightManager.enviormentDiffContributionUI();
		}
		ImGui::EndGroup();

		if (!*procederualSkyToggle)
		{
			m_fileDialog.Display();
			if (m_fileDialog.HasSelected())
			{
				std::string strPath = m_fileDialog.GetSelected().string();
				size_t pos = strPath.find("Textures");
				strPath.erase(0, pos);
				std::wstring path = charToWchar(strPath);
				switch (m_loadNewCubemapType)
				{
				case CubemapType::Skybox:
					m_sky.updateSkyCubemap(path);
					m_sky.cubemapPreviewsRenderSetup();
					m_deviceContext->Draw(4, 0);
					m_loadNewCubemapType = CubemapType::None;
					break;
				case CubemapType::Irradiance:
					m_sky.updateIrradianceCubemap(path);
					m_sky.cubemapPreviewsRenderSetup();
					m_deviceContext->Draw(4, 0);
					m_loadNewCubemapType = CubemapType::None;
					break;
				default:
					break;
				}
			}
			m_fileDialog.ClearSelected();
		}
		ImGui::Unindent(indentSize);
	}
}

void RenderHandler::render(double dt)
{
	// Clear Frame
	
	// - HDR, Output and DepthStencil
	m_deviceContext->ClearRenderTargetView(m_hdrRTV.rtv, clearColor);
	m_deviceContext->ClearRenderTargetView(m_outputRTV.Get(), clearColor);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// - G-Buffer
	ID3D11RenderTargetView* renderTargets[] = {
		m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC].rtv,
		m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].rtv,
		m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK].rtv,
		m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].rtv
	};

	for (int i = 0; i < GBufferType::GB_NUM - 2; i++)
		m_deviceContext->ClearRenderTargetView(renderTargets[i], clearColorBlack);
	m_deviceContext->ClearRenderTargetView(renderTargets[GBufferType::GB_NUM - 2], clearColorWhite); // Clear Shadow Mask

	// - Volumetric Accumulation 
	m_deviceContext->ClearRenderTargetView(m_volumetricAccumulationRTV.rtv, clearColorBlack);

	// - Bloom Textures
	for (UINT i = 0; i < NR_OF_BLOOM_BUFFERS; i++)
		for (UINT j = 0; j < NR_OF_BLOOM_MIPS; j++)
			m_deviceContext->ClearUnorderedAccessViewFloat(m_bloomMipUAVs[i][j].Get(), clearColorBlack);

	// - Adaptive Exposure Histogram
	m_deviceContext->ClearUnorderedAccessViewUint(m_histogramUAV.Get(), clearBlackUint);
	/*m_deviceContext->ClearUnorderedAccessViewFloat(m_histogramImageTexture.uav, clearColorBlack);*/

	// Render Shadow Map
	if (m_shadowMappingEnabled)
	{
		m_shadowInstance.bindViewsAndRenderTarget(); // Also sets Shadow Comparison Sampler

		for (auto& object : m_renderObjects)
			object.second->render(true);
		for (auto& object : m_renderObjectsPBR)
			object.second->render(true);
	}
	else
		m_shadowInstance.clearShadowMap();

	// Set Viewport
	m_deviceContext->RSSetViewports(1, &m_viewport);

	// Set Depth Stencil State
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	// Set Default Render States
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	UINT sampleMask = 0xffffffff;
	m_deviceContext->OMSetBlendState(m_blendStateBlend.Get(), blendFactor, sampleMask);
	if (m_wireframeMode)
		m_deviceContext->RSSetState(m_wireframeRasterizerState.Get());
	else
		m_deviceContext->RSSetState(m_defaultRasterizerState.Get());
	
	// Set G-Buffer Render Targets
	m_deviceContext->OMSetRenderTargets(GBufferType::GB_NUM - 1, renderTargets, m_depthStencilView.Get());
	
	// Draw
	
	// - PHONG
	m_deviceContext->PSSetShaderResources(3, 1, m_shadowInstance.getShadowMapSRV()); // 3th register slot in PHONG Pixel Shader
	m_shaderStates[ShaderStates::PHONG].setShaders();
	for (auto &object : m_renderObjects)
		object.second->render(true);

	// - Light Indicators
	m_lightManager.renderLightIndicators();
	
	// - PBR
	m_deviceContext->PSSetShaderResources(5, 1, &m_shaderResourceNullptr); // 6th register slot in PBR Pixel Shader
	m_deviceContext->PSSetShaderResources(6, 1, m_shadowInstance.getShadowMapSRV()); // 6th register slot in PBR Pixel Shader
	m_shaderStates[ShaderStates::PBR].setShaders();
	for (auto& object : m_renderObjectsPBR)
		object.second->render(true);

	// Volumetric Sun Scattering
	m_sky.setSkyLight(); // Used by Light Pass and Proceural Skybox Shader too
	if (m_volumetricSunToggle)
		volumetricSunPass();

	// SSAO
	if (m_ssaoToggle)
	{
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, nullptr);
		m_deviceContext->PSSetShaderResources(0, 1, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv);
		m_deviceContext->PSSetShaderResources(1, 1, &m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv);
		
		if (m_useHBAOToggle)
			m_HBAOInstance.render();
		else
			m_SSAOInstance.render();

		// Blur
		if (m_ssaoBlurToggle)
			blurSSAOPass();
	}
	
	// Light Pass
	lightPass();

	// Re-Set Render Target with Deph Buffer
	m_deviceContext->OMSetRenderTargets(1, &m_hdrRTV.rtv, m_depthStencilView.Get());

	// Skybox
	m_sky.render();

	// Particles
	particlePass();

	// Lens Flare
	if (m_lensFlareToggle)
	{
		m_deviceContext->OMSetBlendState(m_blendStatePreMultipliedAlphaBlend.Get(), blendFactor, sampleMask);

		m_deviceContext->OMSetRenderTargets(1, &m_hdrRTV.rtv, nullptr);
		m_deviceContext->GSSetShaderResources(1, 1, &m_gBuffer.renderTextures[DEPTH].srv);
		m_deviceContext->PSSetShaderResources(0, 1, &m_lensFlareTexturesSRV);
		m_lensFlareShaders.setShaders();
		m_deviceContext->Draw(9, 0); // 9 quads will be generated with each of the 9 lens flare images

		// - Reset
		m_deviceContext->GSSetShaderResources(1, 1, &m_shaderResourceNullptr);
		m_deviceContext->OMSetBlendState(m_blendStateBlend.Get(), blendFactor, sampleMask);
	}

	// Draw Selection Indicators
	if (m_selectedObjectKey.valid)
	{
		// - Wireframe
		m_deviceContext->RSSetState(m_wireframeRasterizerState.Get()); // Wireframe On
		m_selectionShaders.setShaders();
		m_deviceContext->PSSetShaderResources(0, 1, &m_shaderResourceNullptr);
		m_deviceContext->PSSetConstantBuffers(4, 1, m_selectionCBuffer.GetAddressOf());
		
		/*float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState(m_blendStateNoBlend.Get(), blendFactor, sampleMask);*/
		
		// Render Wireframe
		switch (m_selectedObjectKey.objectType)
		{
		case ShaderStates::PHONG:
			m_renderObjects[m_selectedObjectKey]->render(true);
			break;
		case ShaderStates::PBR:
			m_renderObjectsPBR[m_selectedObjectKey]->render(true);
			break;
		default:
			break;
		}
		
		m_deviceContext->RSSetState(m_defaultRasterizerState.Get()); // Wireframe Off
		// - Arrows
		m_modelSelectionHandler.renderArrows();
	}

	// Adaptive Exposure
	if (m_adaptiveExposureToggle)
		adaptiveExposurePass((float)dt);

	// Bloom
	if (m_bloomToggle)
		bloomPass();

	// Tonemapping
	m_deviceContext->OMSetRenderTargets(1, m_outputRTV.GetAddressOf(), nullptr);
	m_tonemapShaders.setShaders();
	m_deviceContext->PSSetShaderResources(0, 1, &m_hdrRTV.srv);
	if (NR_OF_BLOOM_MIPS % 2 == 0)
		m_deviceContext->PSSetShaderResources(1, 1, &m_bloomBuffers[FirstPingPong].srv);
	else
		m_deviceContext->PSSetShaderResources(1, 1, &m_bloomBuffers[SecondPingPong].srv);
	
	if (m_adaptiveExposureToggle)
		m_deviceContext->PSSetShaderResources(2, 1, m_luminanceSRV.GetAddressOf());
	else
	{
		auto* greyTexture = ResourceHandler::getInstance().getTexture(L"DefaultGrey.jpg");
		m_deviceContext->PSSetShaderResources(2, 1, &greyTexture);
	}

	m_deviceContext->PSSetConstantBuffers(0, 1, m_tonemapCBuffer.GetAddressOf());
	m_deviceContext->Draw(4, 0);

	// Unbind
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
	m_deviceContext->PSSetShaderResources(0, 5, m_shaderResourcesNullptr);

	// ImGUI
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap Frames
	m_swapChain->Present(0, 0);
}