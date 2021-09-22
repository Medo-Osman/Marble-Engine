#include "pch.h"
#include "RenderHandler.h"

RenderHandler::RenderHandler()
{
	m_window = nullptr;
	m_settings = nullptr;
	m_wireframeMode = false;

	m_viewport = D3D11_VIEWPORT();
	m_animationDirection = 1.f;
}

RenderHandler::~RenderHandler()
{
	delete m_blurConstantData;
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

void RenderHandler::initRenderTarget(RenderTexture& rtv, UINT width, UINT height)
{
	// Texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = rtv.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

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
	srvDesc.Texture2D.MipLevels = 1;

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

	// Create Depth Stencil State
	hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
	assert(SUCCEEDED(hr) && "Error, failed to create depth stencil state!");

	// Create Disabled Depth Stencil State
	dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
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

	HRESULT hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_defaultRasterizerState);
	assert(SUCCEEDED(hr) && "Error, failed to create default rasterizer state!");

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_wireframeRasterizerState);
	assert(SUCCEEDED(hr) && "Error, failed to create wireframe rasterizer state!");

	// Sampler State Setup
	D3D11_SAMPLER_DESC samplerStateDesc;
	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.MinLOD = (-FLT_MAX);
	samplerStateDesc.MaxLOD = (FLT_MAX);
	samplerStateDesc.MipLODBias = 0.0f;
	samplerStateDesc.MaxAnisotropy = 1;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerStateDesc.BorderColor[0] = 1.f;
	samplerStateDesc.BorderColor[1] = 1.f;
	samplerStateDesc.BorderColor[2] = 1.f;
	samplerStateDesc.BorderColor[3] = 1.f;

	hr = m_device->CreateSamplerState(&samplerStateDesc, &m_defaultSamplerState);
	assert(SUCCEEDED(hr) && "Error, failed to create default sampler state!");

	// Blend State
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState(&blendStateDesc, m_blendStateNoBlend.GetAddressOf());

	blendStateDesc.RenderTarget[0].BlendEnable = false;
	//blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
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
	shaderFiles.vs = L"";
	shaderFiles.ps = L"";
	shaderFiles.cs = L"EdgePreservingBlurCS.hlsl";
	m_edgePreservingBlurCS.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS);

	m_blurConstantData->projectionMatrix = m_camera.getProjectionMatrix();
	calculateBlurWeights(m_blurConstantData, MAX_BLUR_RADIUS, m_ssaoBlurSigma);

	m_blurDirectionBuffer.initialize(m_device.Get(), m_deviceContext.Get(), m_blurConstantData, BufferType::CONSTANT);

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
	float weightLength = 2 * blurRadius + 1;
	for (int i = 0; i < weightLength; ++i)
		bufferData->weights[i] /= weightSum;
}

void RenderHandler::lightPass()
{
	// Set Output Render Target
	m_deviceContext->OMSetRenderTargets(1, m_outputRTV.GetAddressOf(), nullptr);

	// Set Camera Buffer
	m_deviceContext->PSSetConstantBuffers(0, 1, m_camera.getConstantBuffer());

	// Set Lights
	m_deviceContext->PSSetConstantBuffers(1, 1, m_lightManager.GetAddressOf());

	// Set G-Buffer Shader Resource views
	ID3D11ShaderResourceView* gBufferSRVs[] =
	{
		m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC].srv,
		m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv,
		m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK].srv,
		m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].srv,
		m_gBuffer.renderTextures[GBufferType::DEPTH].srv
	};
	
	if (m_ssaoToggle)
		gBufferSRVs[GBufferType::AMBIENT_OCCLUSION] = m_SSAOInstance.getSSAORenderTexture().srv;

	m_deviceContext->PSSetShaderResources(0, GBufferType::GB_NUM, gBufferSRVs);
	UINT srvIndex = GBufferType::GB_NUM; // 4

	// Set Specular radiance and Diffuse irradiance maps
	m_skybox.setSkyboxTextures(srvIndex, srvIndex + 1);

	// Set Light Pass Shaders
	m_lightPassShaders.setShaders();

	// Draw Fullscreen Quad
	m_deviceContext->Draw(4, 0);

	// Unbind Render Target and Shader Resource Views
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, nullptr);
	m_deviceContext->PSSetShaderResources(0, 5, m_shaderResourcesNullptr);
}

void RenderHandler::downsampleSSAOPass()
{
	/*m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, NULL);
	m_downsampleCS.setShaders();

	m_deviceContext->CSSetShaderResources(0, 1, &m_SSAOInstance.getSSAORenderTexture().srv);
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

	ID3D11ShaderResourceView* blurSRVs[] = { m_SSAOInstance.getSSAORenderTexture().srv, m_blurPingPongSRV.Get() };
	ID3D11UnorderedAccessView* blurUAVs[] = { m_blurPingPongUAV.Get(), m_SSAOInstance.getSSAORenderTexture().uav };

	for (UINT i = 0; i < 2; i++)
	{
		// Blur Constant Buffer
		m_blurConstantData->direction = i;
		
		m_blurDirectionBuffer.update(m_blurConstantData);
		m_deviceContext->CSSetConstantBuffers(0, 1, m_blurDirectionBuffer.GetAddressOf());

		// Set Rescources
		m_deviceContext->CSSetShaderResources(0, 1, &blurSRVs[m_blurConstantData->direction]);
		m_deviceContext->CSSetShaderResources(1, 1, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv);
		m_deviceContext->CSSetShaderResources(2, 1, &m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv);
		m_deviceContext->CSSetUnorderedAccessViews(0, 1, &blurUAVs[m_blurConstantData->direction], &cOffset);

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
	ResourceHandler::getInstance().setDevice(m_device.Get());

	// Lighting
	m_lightManager.initialize(m_device.Get(), m_deviceContext.Get(), m_camera.getViewMatrixPtr(), m_camera.getProjectionMatrixPtr());
	m_shadowInstance.initialize(m_device.Get(), m_deviceContext.Get(), SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	
	// Skybox
	m_skybox.initialize(m_device.Get(), m_deviceContext.Get(), L"Textures/TableMountain1Cubemap.dds", L"Textures/TableMountain1Cubemap.dds");

	// Timer
	m_timer.restart();

	// Shader States
	m_shaderStates.resize(ShaderStates::NUM);
	ShaderFiles shaderFiles;
	// - Phong
	shaderFiles.vs = L"GeneralVS.hlsl";
	shaderFiles.ps = L"GBufferPS.hlsl";
	//shaderFiles.ps = L"GeneralPS.hlsl";
	m_shaderStates[ShaderStates::PHONG].initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);
	// - PBR
	shaderFiles.vs = L"GeneralVS.hlsl";
	shaderFiles.ps = L"GBufferPBR_PS.hlsl";
	//shaderFiles.ps = L"PBR_PS.hlsl";
	m_shaderStates[ShaderStates::PBR].initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);

	// - Light pass Shaders
	shaderFiles.vs = L"FullscreenQuadVS.hlsl";
	shaderFiles.ps = L"LightPassPS.hlsl";
	m_lightPassShaders.initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS);

	// SSAO
	m_HBAOInstance.initialize(m_device.Get(), m_deviceContext.Get(), winRect.right, winRect.bottom, m_camera.getFarZ(), m_camera.getFov(), m_camera.getViewMatrix(), m_camera.getProjectionMatrix());
	m_SSAOInstance.initialize(m_device.Get(), m_deviceContext.Get(), winRect.right, winRect.bottom, m_camera.getFarZ(), m_camera.getFov(), m_camera.getViewMatrix(), m_camera.getProjectionMatrix());

	// Blur
	initSSAOBlurPass(m_clientWidth, m_clientHeight, m_SSAOInstance.getSSAORenderTexture().format);
	
	// Particles
	m_particleSystem.Initialize(m_device.Get(), m_deviceContext.Get(), L"flare.dds", 10);

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
}

void RenderHandler::updateCamera(XMVECTOR position, XMVECTOR rotation)
{
	m_camera.updateViewMatrix(position, rotation);
	m_skybox.updateVP(m_camera.getViewMatrix(), m_camera.getProjectionMatrix());
	//m_HBAOInstance.updateViewMatrix(m_camera.getViewMatrix());
	m_SSAOInstance.updateViewMatrix(m_camera.getViewMatrix());
	m_shadowInstance.buildLightMatrix(m_camera.getCameraPositionF3());
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
	return RenderObjectKey();
}

void RenderHandler::modelTextureUIUpdate(RenderObjectKey key)
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

int RenderHandler::addLight(Light newLight, bool usedForShadowMapping)
{
	if (m_lightManager.addLight(newLight))
	{
		m_lightManager.update();
		if (usedForShadowMapping && newLight.type == DIRECTIONAL_LIGHT)
			m_shadowInstance.buildLightMatrix(newLight, m_camera.getCameraPositionF3());

		return m_lightManager.getNrOfLights(); // Used as a ID
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
}

void RenderHandler::changeShadowMappingLight(Light* light, bool disableShadowCasting)
{
	if (disableShadowCasting)
	{
		m_shadowMappingEnabled = false;
		m_shadowInstance.clearShadowMap();
	}
	else
	{
		m_shadowMappingEnabled = true;
		XMVECTOR normDirection = XMLoadFloat4(&light->direction);
		XMVector4Normalize(normDirection);
		XMStoreFloat4(&light->direction, normDirection);
		m_shadowInstance.buildLightMatrix(*light, m_camera.getCameraPositionF3());
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

void RenderHandler::update(float dt)
{
	// Particles
	m_particleSystem.update(dt, (float)m_timer.timeElapsed(), m_camera);

	// Selection
	if (m_selectedObjectKey.valid)
	{
		m_selectionAnimationData.colorOpacity += (m_animationDirection * dt);
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

void RenderHandler::updateShaderState(ShaderStates shaderState)
{
	m_shaderStates[shaderState].updateShaders();
}

void RenderHandler::updatePassShaders()
{
	m_lightPassShaders.updateShaders();
	//m_HBAOInstance.updateShaders();
	m_SSAOInstance.updateShaders();
	m_edgePreservingBlurCS.updateShaders();
}

void RenderHandler::UIRenderShadowMap()
{
	ImGui::Begin("Shadow Map");
	ImGui::Image(m_shadowInstance.getShadowMapSRVNoneConst(), ImVec2(400.f, 400.f));
	ImGui::End();
}

void RenderHandler::UIRenderPipelineTexturesWindow()
{
	ImGui::Begin("Horizon Based Ambient Occlusion Texture");
	
	static ImVec4 color_multipler(1, 1, 1, 100);
	ImGui::Image(m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	ImGui::Image(m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	ImGui::Image(m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	//ImGui::Image(m_gBuffer.renderTextures[GBufferType::DEPTH].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	//ImGui::Image(m_shadowInstance.getShadowMapSRVNoneConst(), ImVec2((float)m_clientWidth / 4.f, (float)m_clientWidth / 4.f));
	
	ImGui::Image(m_blurPingPongSRV.Get(), ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	if (m_ssaoToggle)
		ImGui::Image(m_SSAOInstance.getSSAORenderTexture().srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	else
		ImGui::Image(m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].srv, ImVec2((float)m_clientWidth / 4.f, (float)m_clientHeight / 4.f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color_multipler);
	
	//m_SSAOInstance.updateUI();
	ImGui::End();
}

void RenderHandler::UIssaoSettings()
{
	if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_DefaultOpen))
	{
		m_ssaoToggle = true;
		ImGui::Indent(16.0f);
		ImGui::PushItemWidth(-16.f);
		if (ImGui::CollapsingHeader("Blur", ImGuiTreeNodeFlags_DefaultOpen))
		{
			m_ssaoBlurToggle = true;
			ImGui::Indent(16.0f);
			ImGui::PushItemWidth(-40.f);
			if (ImGui::DragFloat("Sigma", &m_ssaoBlurSigma, 0.1f, 1.f, 5.f))
				calculateBlurWeights(m_blurConstantData, MAX_BLUR_RADIUS, m_ssaoBlurSigma);

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

void RenderHandler::render()
{
	// Clear Frame
	m_deviceContext->ClearRenderTargetView(m_outputRTV.Get(), clearColor);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* renderTargets[] = {
		m_gBuffer.renderTextures[GBufferType::ALBEDO_METALLIC].rtv,
		m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].rtv,
		m_gBuffer.renderTextures[GBufferType::EMISSIVE_SHADOWMASK].rtv,
		m_gBuffer.renderTextures[GBufferType::AMBIENT_OCCLUSION].rtv
	};

	for (int i = 0; i < GBufferType::GB_NUM - 2; i++)
		m_deviceContext->ClearRenderTargetView(renderTargets[i], clearColorBlack);
	m_deviceContext->ClearRenderTargetView(renderTargets[GBufferType::GB_NUM - 2], clearColorWhite); // Clear Shadow Mask

	// Render Shadow Map
	if (m_shadowMappingEnabled)
	{
		m_shadowInstance.bindViewsAndRenderTarget(); // Also sets Shadow Comparison Sampler

		for (auto& object : m_renderObjects)
			object.second->render(true);
		for (auto& object : m_renderObjectsPBR)
			object.second->render(true);
	}
	
	// Set Render Target
	m_deviceContext->OMSetRenderTargets(1, m_outputRTV.GetAddressOf(), m_depthStencilView.Get());

	// Set Viewport
	m_deviceContext->RSSetViewports(1, &m_viewport);

	// Set Depth Stencil State
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	// Set Default Render States
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT sampleMask = 0xffffffff;
	m_deviceContext->OMSetBlendState(m_blendStateBlend.Get(), blendFactor, sampleMask);
	m_deviceContext->PSSetSamplers(0, 1, m_defaultSamplerState.GetAddressOf());
	if (m_wireframeMode)
		m_deviceContext->RSSetState(m_wireframeRasterizerState.Get());
	else
		m_deviceContext->RSSetState(m_defaultRasterizerState.Get());
	
	// Set G-Buffer Render Targets
	m_deviceContext->OMSetRenderTargets(GBufferType::GB_NUM - 1, renderTargets, m_depthStencilView.Get());
	
	// Draw
	
	// - PHONG
	m_deviceContext->PSSetShaderResources(3, 1, m_shadowInstance.getShadowMapSRV()); // 6th register slot in PBR Pixel Shader
	m_shaderStates[ShaderStates::PHONG].setShaders();
	for (auto &object : m_renderObjects)
		object.second->render(true);

	// - Light Indicators
	m_lightManager.renderLightIndicators();
	
	// - PBR
	m_deviceContext->PSSetShaderResources(6, 1, m_shadowInstance.getShadowMapSRV()); // 6th register slot in PBR Pixel Shader
	m_shaderStates[ShaderStates::PBR].setShaders();
	for (auto& object : m_renderObjectsPBR)
		object.second->render(true);

	m_deviceContext->RSSetState(m_defaultRasterizerState.Get());

	// SSAO
	if (m_ssaoToggle)
	{
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetNullptr, nullptr);
		m_deviceContext->PSSetShaderResources(0, 1, &m_gBuffer.renderTextures[GBufferType::DEPTH].srv);
		m_deviceContext->PSSetShaderResources(1, 1, &m_gBuffer.renderTextures[GBufferType::NORMAL_ROUGNESS].srv);
		//m_HBAOInstance.render();
		m_SSAOInstance.render();

		// Downsample SSAO Texture
		downsampleSSAOPass();

		// Blur
		if (m_ssaoBlurToggle)
			blurSSAOPass();
	}
	
	// Light Pass
	lightPass();

	// Re-Set Render Target
	m_deviceContext->OMSetRenderTargets(1, m_outputRTV.GetAddressOf(), m_depthStencilView.Get());

	// Skybox
	m_skybox.render();

	// Draw Particles
	/*m_deviceContext->OMSetDepthStencilState(m_disabledDepthStencilState.Get(), 0);
	m_particleSystem.render();*/

	// Draw Selection Indicators
	if (m_selectedObjectKey.valid)
	{
		// - Wireframe
		m_deviceContext->RSSetState(m_wireframeRasterizerState.Get()); // Wireframe On
		m_selectionShaders.setShaders();
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_deviceContext->PSSetShaderResources(0, 1, &nullSRV);
		m_deviceContext->PSSetConstantBuffers(4, 1, m_selectionCBuffer.GetAddressOf());
		
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState(m_blendStateNoBlend.Get(), blendFactor, sampleMask);
		
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
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
	m_deviceContext->OMSetRenderTargets(1, m_outputRTV.GetAddressOf(), nullptr);
	m_deviceContext->PSSetShaderResources(0, 5, m_shaderResourcesNullptr);

	// ImGUI
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap Frames
	m_swapChain->Present(0, 0);
}