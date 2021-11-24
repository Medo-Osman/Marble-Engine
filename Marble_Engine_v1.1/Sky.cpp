#include "pch.h"
#include "Sky.h"

void Sky::initPreviewTexture(ID3D11Device* device, RenderTexture& rtv)
{
	// Texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = m_previewTextureWidth;
	textureDesc.Height = m_previewTextureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = m_SkyPreviewTexture.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, NULL, &rtv.rtt);
	assert(SUCCEEDED(hr) && "Error, render target texture could not be created!");

	// Render Rarget View
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hr = device->CreateRenderTargetView(rtv.rtt, &renderTargetViewDesc, &rtv.rtv);
	assert(SUCCEEDED(hr) && "Error, render target view could not be created!");

	// Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(rtv.rtt, &srvDesc, &rtv.srv);
	assert(SUCCEEDED(hr) && "Error, shader resource view could not be created!");
}

void Sky::updateSkyLight()
{
	// Direction
	m_lightRotation = XMLoadFloat3(&m_skyLightData.direction);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(-XM_PIDIV2, XMConvertToRadians((-m_timeOfDay * 360.f) - m_lightRotationOffset), 0);
	XMVECTOR rotQuatInverse = XMQuaternionInverse(rotQuat);
	XMVECTOR lightDir = XMQuaternionMultiply(rotQuat, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	lightDir = XMQuaternionMultiply(lightDir, rotQuatInverse);

	XMStoreFloat3(&m_skyLightData.direction, lightDir);
	m_sunLight.direction = m_skyLightData.direction;
	XMStoreFloat3(&m_moonLight.direction, -lightDir);

	// Sun Set/Rise Color
	float SunSetRiseRadius = 0.4f;
	float sunSetRiseRange = std::abs(std::abs(remap(m_timeOfDay, XMFLOAT2(0.f, 1.f), XMFLOAT2(-1.f, 1.f))) - 0.5f) * 2.f; // remaps to 0 - 1 - 0
	
	if (sunSetRiseRange < SunSetRiseRadius)
		sunSetRiseRange = (SunSetRiseRadius - sunSetRiseRange);
	else
		sunSetRiseRange = 0.f;
	sunSetRiseRange = smoothstep(0.f, SunSetRiseRadius, sunSetRiseRange);

	// Day/Night
	if (m_timeOfDay > 0.25f && m_timeOfDay < 0.75f)
	{
		XMStoreFloat3(&m_skyLightData.color, XMVectorLerp(XMLoadFloat3(&m_sunLight.color), XMLoadFloat3(&m_proceduralSkyData.sunSetRiseColor), sunSetRiseRange));
		m_skyLightData.intensity = m_sunLight.intensity;

		m_skyLightData.moonOrSun = SUN;
		m_shadowmapInstance->buildLightMatrix(m_sunLight);
	}
	else
	{
		m_skyLightData.color = m_moonLight.color;
		m_skyLightData.intensity = m_moonLight.intensity;
		m_skyLightData.moonOrSun = MOON;
		m_shadowmapInstance->buildLightMatrix(m_moonLight);
	}

	// Update Constant Buffer
	m_skyLightCBuffer.update(&m_skyLightData);
}

void Sky::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ShadowMapInstance* shadowmapInstance, Light& sunLight, Light& moonLight, std::wstring SkyTexturePath, std::wstring irradianceTexturePath, XMFLOAT3 procedSkyColor, XMFLOAT3 procedGroundColor)
{
	// Device
	m_deviceContext = deviceContext;

	// Shadowmap Instance
	m_shadowmapInstance = shadowmapInstance;

	// Shaders
	ShaderFiles shaders;
	shaders.vs = L"SkyboxVS.hlsl";
	shaders.ps = L"SkyboxPS.hlsl";
	m_skyboxShaders.initialize(device, m_deviceContext, shaders, LayoutType::POS);

	shaders.vs = L"FullscreenQuadVS.hlsl";
	shaders.ps = L"SkyboxPreviewPS.hlsl";
	m_previewShaders.initialize(device, m_deviceContext, shaders, LayoutType::POS);

	shaders.vs = L"ProceduralSkyVS.hlsl";
	shaders.ps = L"ProceduralSkyPS.hlsl";
	m_proceduralShaders.initialize(device, m_deviceContext, shaders, LayoutType::POS);

	// Cube
	std::vector<VertexPos> vertices =
	{
		// Back
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f) },

		// Front
		{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },

		// Left
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },

		// Right
		{ XMFLOAT3(1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f) },

			// Bottom
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f) },

			// Top
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f) }
	};

	// Mesh
	PS_MATERIAL_BUFFER material;
	std::vector<UINT> indices;
	m_cubeMesh = new Mesh<VertexPos>(device, m_deviceContext, vertices, indices, material, TexturePaths());
	m_sphereModel.initialize(device, m_deviceContext, 999547, "inverted_sphere.glb");

	// Constant Buffer
	m_vpCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);

	m_proceduralSkyData.skyColor = procedSkyColor;
	m_proceduralSkyData.groundColor = procedGroundColor;
	m_proceduralSkyCBuffer.initialize(device, deviceContext, &m_proceduralSkyData, BufferType::CONSTANT);

	m_proceduralVPMatrixCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);

	m_skyLightCBuffer.initialize(device, deviceContext, nullptr, BufferType::CONSTANT);

	// Texture

	// - Sky Texture
	m_SkyTexturePath = SkyTexturePath;
	m_SkyTextureSRV = ResourceHandler::getInstance().getTexture(SkyTexturePath.c_str());
	initPreviewTexture(device, m_SkyPreviewTexture);

	// - Irradiance Texture
	m_irradianceTexturePath = irradianceTexturePath;
	m_irradianceTextureSRV = ResourceHandler::getInstance().getTexture(irradianceTexturePath.c_str());
	initPreviewTexture(device, m_irradiancePreviewTexture);

	// - Preivew Viewport
	m_previewViewport.TopLeftX = 0.f;
	m_previewViewport.TopLeftY = 0.f;
	m_previewViewport.Width = (FLOAT)m_previewTextureWidth;
	m_previewViewport.Height = (FLOAT)m_previewTextureHeight;
	m_previewViewport.MinDepth = 0.f;
	m_previewViewport.MaxDepth = 1.f;

	// - Noise
	m_starsSRV = ResourceHandler::getInstance().getTexture(L"starry_sky.png");

	// - Moon
	m_moonSRV = ResourceHandler::getInstance().getTexture(L"noise_clouds.png");

	// - Blue Noise
	m_blueNoiseSRV = ResourceHandler::getInstance().getTexture(L"blue_noise_small.png");

	// Constant Buffers
	m_vpCBuffer.initialize(device, m_deviceContext, nullptr, BufferType::CONSTANT);

	// Rotation
	//m_rotation = XMVectorSet(0.f, XMConvertToRadians(160.f), 0.f, 0.f);

	m_sunLight = sunLight;
	m_moonLight = moonLight;
	m_skyLightData.direction = XMFLOAT3(0.f, 1.f, 0.f);
	m_skyLightData.castingShadow = true;
	m_dayNightCycleToggle = true;
	m_timeOfDay = m_timeOffset; // 0 = 00:00(Midnight), 0.25 = 06:00(Morning), 0.5 = 12:00(Noon),  0.75 = 18:00(Evening)
	m_cyclePerMinute = 0.1f;
	m_dayNightTimer.shouldRememberDuringPause(true);
	m_dayNightTimer.start();
}

Sky::Sky()
{
	m_deviceContext = nullptr;
	m_rotation = XMVectorZero();
}
Sky::~Sky()
{
	delete m_cubeMesh;
}

void Sky::updateUI()
{
	// Sun / Moon Color and Intensity
	ImGui::ColorEdit3("Sun Color", &m_sunLight.color.x, ImGuiColorEditFlags_Float);
	ImGui::DragFloat("Sun Intensity", &m_sunLight.intensity, 0.01f, 0.f, 200.f);

	if (ImGui::ColorEdit3("Sun Set/Rise Color", &m_proceduralSkyData.sunSetRiseColor.x, ImGuiColorEditFlags_Float))
		updateProceduralData();

	if (ImGui::ColorEdit3("Moon Color", &m_moonLight.color.x, ImGuiColorEditFlags_Float))
	{
		m_proceduralSkyData.moonColor = m_moonLight.color;
		updateProceduralData();
	}

	ImGui::DragFloat("Moon Intensity", &m_moonLight.intensity, 0.01f, 0.f, 200.f);
	ImGui::Separator();

	// Day Night Cycle
	ImGui::Text("Day Night Cycle");
	ImGui::DragFloat("Cycles Per Minute", &m_cyclePerMinute, 0.01f, 0.01f, 100.f);

	if (ImGui::SliderFloat("Time Of Day", &m_timeOfDay, 0.f, 1.f))
		updateSkyLight();

	if (ImGui::Checkbox("Run Cycle", &m_dayNightCycleToggle))
	{
		if (m_dayNightCycleToggle)
			m_dayNightTimer.start();
		else
			m_dayNightTimer.stop();
	}
}

void Sky::updateSkyCubemap(std::wstring path)
{
	m_SkyTextureSRV = ResourceHandler::getInstance().getTexture(path.c_str());
	m_SkyTexturePath = path;
}

void Sky::updateIrradianceCubemap(std::wstring path)
{
	m_irradianceTextureSRV = ResourceHandler::getInstance().getTexture(path.c_str());
	m_irradianceTexturePath = path;
}

void Sky::updateProceduralData()
{
	m_proceduralSkyCBuffer.update(&m_proceduralSkyData);
}

void Sky::updatePreviewShaders()
{
	m_previewShaders.updateShaders();
}

void Sky::updateProceduralShaders()
{
	m_proceduralShaders.updateShaders();
}

void Sky::updateMatrices(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, const XMVECTOR& camPosition)
{
	if (m_proceduralToggle)
	{
		m_proceduralVPMatrixData.viewMatrix = XMMatrixTranspose(viewMatrix);
		m_proceduralVPMatrixData.projMatrix = XMMatrixTranspose(projectionMatrix);
		m_proceduralVPMatrixCBuffer.update(&m_proceduralVPMatrixData);
	}
	else
	{
		VS_SKYBOX_MATRIX_CBUFFER* vpData = new VS_SKYBOX_MATRIX_CBUFFER();

		XMMATRIX worldMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation); // Rotation only
		vpData->vpMatrix = XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);
		m_vpCBuffer.update(&vpData);
	}
}

void Sky::update(double dt)
{
	if (m_dayNightCycleToggle)
	{
		float time = (float)m_dayNightTimer.timeElapsed();
		m_timeOfDay = std::fmod(m_timeOffset + time * (6.f / m_cyclePerMinute / 360.f), 1.f);

		/*OutputDebugStringA(std::to_string(m_timeOfDay).c_str());
		OutputDebugStringA("\n");*/

		updateSkyLight();

		/*if (m_timeOfDay > 0.25 && m_timeOfDay < 0.5f)
		{
			OutputDebugString(L"Morning\n");
		}
		else if (m_timeOfDay > 0.5f && m_timeOfDay < 0.75f)
		{
			OutputDebugString(L"Noon\n");
		}
		else if (m_timeOfDay > 0.75f)
		{
			OutputDebugString(L"Evening\n");
		}
		else
		{
			OutputDebugString(L"Night\n");
		}*/
	}
}