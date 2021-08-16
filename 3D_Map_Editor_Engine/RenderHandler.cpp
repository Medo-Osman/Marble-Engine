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

	// Back Buffer Texture
	ID3D11Texture2D* backBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	assert(SUCCEEDED(hr) && "Error, failed to set backbuffer!");

	// Render Targets
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
	m_viewport.Width = (FLOAT)m_settings->width;
	m_viewport.Height = (FLOAT)m_settings->height;
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

	hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
	assert(SUCCEEDED(hr) && "Error, failed to create depth stencil state!");

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

void RenderHandler::initCamera()
{
	m_camera.initialize(m_device.Get(), m_deviceContext.Get(), m_settings->fov, (float)m_settings->width / (float)m_settings->height, 0.1f, 1000.f);
}

void RenderHandler::initialize(HWND* window, Settings* settings)
{
	m_window = window;
	m_settings = settings;

	initDeviceAndSwapChain();
	initViewPort();
	initDepthStencilBuffer();
	initRenderStates();
	initCamera();
	ImGui_ImplDX11_Init(m_device.Get(), m_deviceContext.Get());
	ResourceHandler::getInstance().setDevice(m_device.Get());

	// Lighting
	m_lightManager.initialize(m_device.Get(), m_deviceContext.Get(), m_camera.getViewMatrixPtr(), m_camera.getProjectionMatrixPtr());

	m_shadowInstance.initialize(m_device.Get(), m_deviceContext.Get(), SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	//m_shadowInstance.buildLightMatrix(light);

	// Skybox
	m_skybox.initialize(m_device.Get(), m_deviceContext.Get(), L"Textures/TableMountain1Cubemap.dds", L"Textures/TableMountain1Cubemap.dds");

	// Timer
	m_timer.restart();

	// Shader States
	m_shaderStates.resize(ShaderStates::NUM);
	ShaderFiles shaderFiles;
	// - Phong
	shaderFiles.vs = L"GeneralVS.hlsl";
	shaderFiles.ps = L"GeneralPS.hlsl";
	m_shaderStates[ShaderStates::PHONG].initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);
	// - PBR
	shaderFiles.vs = L"GeneralVS.hlsl";
	shaderFiles.ps = L"PBR_PS.hlsl";
	m_shaderStates[ShaderStates::PBR].initialize(m_device.Get(), m_deviceContext.Get(), shaderFiles, LayoutType::POS_NOR_TEX_TAN);

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
	key.key = objects->size();
	key.valid = true;
	key.objectType = shaderState;

	(*objects)[key] = new RenderObject();
	objects->at(key)->initialize(m_device.Get(), m_deviceContext.Get(), objects->size(), modelName);
	if (m_camera.isInitialized())
	{
		XMMATRIX viewProjMatrix = m_camera.getViewMatrix() * m_camera.getProjectionMatrix();
		objects->at(key)->updateWCPBuffer(XMMatrixIdentity(), viewProjMatrix);
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

void RenderHandler::updateRenderObjectWorld(RenderObjectKey key, XMMATRIX worldMatrix)
{
	XMMATRIX viewProjMatrix = m_camera.getViewMatrix() * m_camera.getProjectionMatrix();
	switch (key.objectType)
	{
	case PHONG:
		m_renderObjects[key]->updateWCPBuffer(worldMatrix, viewProjMatrix);
		break;
	case PBR:
		m_renderObjectsPBR[key]->updateWCPBuffer(worldMatrix, viewProjMatrix);
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
				newKey.key = m_renderObjectsPBR.size();
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
				newKey.key = m_renderObjects.size();
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
}

int RenderHandler::addLight(Light newLight, bool usedForShadowMapping)
{
	if (m_lightManager.addLight(newLight))
	{
		m_lightManager.update();
		if (usedForShadowMapping && newLight.type == DIRECTIONAL_LIGHT)
			m_shadowInstance.buildLightMatrix(newLight);

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
		m_shadowInstance.buildLightMatrix(*light);
	}
}

bool* RenderHandler::getWireframeModePtr()
{
	return &m_wireframeMode;
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
	XMMATRIX view = m_camera.getViewMatrix();
	XMMATRIX proj = m_camera.getProjectionMatrix();
	float fov = m_camera.getFov();
	float aspectRatio = (float)m_settings->width / (float)m_settings->height;

	// Projection conversion
	float projectionX = 1 / (aspectRatio * tan(fov / 2));
	float projectionY = 1 / tan(fov / 2);
	float viewX = (2.f * pointX / m_settings->width - 1.f) / projectionX;
	float viewY = (-2.f * pointY / m_settings->height + 1.f) / projectionY;
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
	XMMATRIX view = m_camera.getViewMatrix();
	XMMATRIX proj = m_camera.getProjectionMatrix();
	float fov = m_camera.getFov();
	float aspectRatio = (float)m_settings->width / (float)m_settings->height;
	
	// Projection conversion
	float projectionX = 1 / (aspectRatio * tan(fov / 2));
	float projectionY = 1 / tan(fov / 2);
	float viewX = (2.f * pointX / m_settings->width - 1.f) / projectionX;
	float viewY = (-2.f * pointY / m_settings->height + 1.f) / projectionY;
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

void RenderHandler::render()
{
	// Clear Frame
	m_deviceContext->ClearRenderTargetView(m_outputRTV.Get(), clearColor);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render Shadow Map
	if (m_shadowMappingEnabled)
	{
		m_shadowInstance.bindViewsAndRenderTarget();

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

	// Set Lights
	m_deviceContext->PSSetConstantBuffers(1, 1, m_lightManager.GetAddressOf());
	m_lightManager.renderLightIndicators();

	// Set Camera Buffer
	m_deviceContext->PSSetConstantBuffers(2, 1, m_camera.getConstantBuffer());

	// Set Shadow Map Texture and Constant Buffer
	m_deviceContext->PSSetShaderResources(3, 1, m_shadowInstance.getShadowMapSRV());
	m_deviceContext->PSSetConstantBuffers(3, 1, m_shadowInstance.getShadowMatrixConstantBuffer());

	// Draw
	// - PHONG
	for (auto &object : m_renderObjects)
		object.second->render();

	// - PBR
	m_deviceContext->PSSetShaderResources(6, 1, m_shadowInstance.getShadowMapSRV()); // 6th register slot in PBR Pixel Shader
	m_shaderStates[ShaderStates::PBR].setShaders();
	m_skybox.setSkyboxTextures(8, 7); // Specular radiance and Diffuse irradiance maps
	for (auto& object : m_renderObjectsPBR)
		object.second->render(true);
	
	ID3D11ShaderResourceView* shaderResourceNullptr = nullptr;
	this->m_deviceContext->PSSetShaderResources(6, 1, &shaderResourceNullptr); // Remove ShadowMap from slot 6

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

	// ImGUI
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap Frames
	m_swapChain->Present(0, 0);
}