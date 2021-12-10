#ifndef SKY_H
#define SKY_H

#include "RenderObject.h"
#include "ShadowMapInstance.h"

enum class CubemapType { Skybox, Irradiance, None };
enum SkyLightType { MOON, SUN, BOTH };

class Sky
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Sky Mode
	bool m_proceduralToggle = true;

	// Texture
	
	// - Sky / Radiance
	std::wstring m_SkyTexturePath;
	ComPtr< ID3D11ShaderResourceView > m_SkyTextureSRV;
	// - Irradiance
	std::wstring m_irradianceTexturePath;
	ComPtr< ID3D11ShaderResourceView > m_irradianceTextureSRV;
	// - Previews
	RenderTexture m_SkyPreviewTexture;
	RenderTexture m_irradiancePreviewTexture;
	D3D11_VIEWPORT m_previewViewport;
	UINT m_previewTextureWidth = 100;
	UINT m_previewTextureHeight = 100;
	// - Stars
	ComPtr< ID3D11ShaderResourceView > m_starsSRV;
	// - Moon
	ComPtr< ID3D11ShaderResourceView > m_moonSRV;
	// - Blue Noise
	ComPtr< ID3D11ShaderResourceView > m_blueNoiseSRV;

	// Mesh
	Mesh<VertexPos>* m_cubeMesh = nullptr;
	Model m_sphereModel;

	// Constant Buffers
	Buffer<VS_SKYBOX_MATRIX_CBUFFER> m_vpCBuffer; // Includes rotation

	VS_VP_MATRIX_CBUFFER m_proceduralVPMatrixData;
	Buffer<VS_VP_MATRIX_CBUFFER> m_proceduralVPMatrixCBuffer;

	PROCEDURAL_SKY_CBUFFER m_proceduralSkyData;
	Buffer<PROCEDURAL_SKY_CBUFFER> m_proceduralSkyCBuffer;

	SKY_LIGHT_DATA_CBUFFER m_skyLightData;
	Buffer< SKY_LIGHT_DATA_CBUFFER > m_skyLightCBuffer;

	// Shaders
	Shaders m_skyboxShaders;
	Shaders m_previewShaders;

	Shaders m_proceduralShaders;

	// Sky Rotation
	XMVECTOR m_rotation;

	// Sun/Moon Light
	Light m_sunLight;
	Light m_moonLight;

	// Day Night Cycle
	XMVECTOR m_lightRotation;
	bool m_dayNightCycleToggle;
	bool m_ambientColorSameAsSunToggle = true;
	float m_timeOffset = 0.25f;
	float m_lightRotationOffset = 180.f;
	float m_timeOfDay; // 0 = 00:00(Midnight), 0.25 = 06:00(Morning), 0.5 = 12:00(Noon),  0.75 = 18:00(Evening)
	float m_cyclePerMinute;
	Timer m_dayNightTimer;
	const float ROTATION_PER_MINUTE = XM_2PI / 60.f;

	// Shadow Instrance, needed to update shadows for Sun/Moon light
	ShadowMapInstance* m_shadowmapInstance;

	// Functions
	void initPreviewTexture(ID3D11Device* device, RenderTexture& rtv);
	void updateSkyLight();

public:
	Sky();
	~Sky();

	// Initialize
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ShadowMapInstance* shadowmapInstance, Light& sunLight, Light& moonLight, std::wstring SkyTexturePath, std::wstring irradianceTexturePath, XMFLOAT3 procedSkyColor = XMFLOAT3(0.05f, 0.09f, 0.2f), XMFLOAT3 procedGroundColor = XMFLOAT3(0.02f, 0.01f, 0.03f));

	// Getters
	std::wstring getSkyFileName() const { return m_SkyTexturePath; }
	std::wstring getIrradianceFileName() const { return m_irradianceTexturePath; }
	ID3D11ShaderResourceView* getSkyPreviewSRV() const { return m_SkyPreviewTexture.srv; }
	ID3D11ShaderResourceView* getIrradiancePreviewSRV() const { return m_irradiancePreviewTexture.srv; }
	PROCEDURAL_SKY_CBUFFER* getProduralSkyDataPtr() { return &m_proceduralSkyData; }
	bool* getProduralSkyTogglePtr() { return &m_proceduralToggle; }
	XMFLOAT3 getSkyLightDirection(SkyLightType types = SkyLightType::BOTH);

	// Setters
	void toggleProceduralSky(bool toggle) { m_proceduralToggle = toggle; }

	// Update
	void updateUI();
	void ambientSettingsUI();
	void updateSkyCubemap(std::wstring path);
	void updateIrradianceCubemap(std::wstring path);
	void updateProceduralData();
	void updatePreviewShaders();
	void updateProceduralShaders();
	void updateMatrices(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, const XMVECTOR& camPosition);
	void update(double dt);

	// Render
	void setSkyLight();
	void setSkyTextures(int SkyRegister, int irradianceRegister);
	void cubemapPreviewsRenderSetup();
	void render();
};

#endif // !SKY_H