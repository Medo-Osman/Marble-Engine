#ifndef SHADOWMAPINSTANCE_H
#define SHADOWMAPINSTANCE_H

#include "Buffer.h"
#include "Shaders.h"

struct VS_SHADOW_C_BUFFER
{
	XMMATRIX lightViewMatrix;
	XMMATRIX lightProjectionMatrix;
};

class ShadowMapInstance
{
private:
	// Device
	ID3D11DeviceContext* m_deviceContext;

	// Dimensions
	UINT m_width;
	UINT m_height;

	// Resources
	ComPtr< ID3D11Texture2D > m_shadowMapTexture;
	ComPtr< ID3D11ShaderResourceView > m_shadowMapSRV;
	ComPtr< ID3D11DepthStencilView > m_shadowMapDSV;

	// Pipeline States
	ComPtr< ID3D11DepthStencilState > m_depthStencilState;
	ComPtr< ID3D11RasterizerState > m_rasterizerState;
	ComPtr< ID3D11SamplerState > m_comparisonSampler;

	// Render Target
	ID3D11RenderTargetView* m_rendertarget[1] = { 0 };

	// Viewport
	D3D11_VIEWPORT m_viewport;

	// Shaders
	Shaders m_shadowMapShaders;

	// Light
	Light m_directionalLight;
	XMVECTOR m_lightPosition;
	XMFLOAT3 m_lightRotationRad;
	XMMATRIX m_invLightViewMatrix;
	XMMATRIX m_invLightProjectionMatrix;

	// World Bounding Sphere
	BoundingSphere m_worldBoundingSphere;
	float m_zOffset;

	// Constant Buffers
	Buffer<VS_SHADOW_C_BUFFER> m_lightMatrixCBuffer;
	Buffer<XMMATRIX> m_invLightVpMatrixCBuffer; // Used for Volumetric Sun Scattering
	Buffer<XMMATRIX> m_shadowTextureMatrixCBuffer;

public:
	ShadowMapInstance();
	~ShadowMapInstance();

	// Initialize
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT width, UINT height);

	// Getters
	Light getLight() const;
	XMMATRIX getInvLightViewMatrix() const;
	XMMATRIX getInvLightProjectionMatrix() const;
	XMVECTOR getLightPosition() const;
	XMVECTOR getLightDirection() const;
	XMVECTOR getLightRotation() const;
	float getLightShadowRadius() const;

	// Update
	void buildLightMatrix(Light directionalLight, XMFLOAT3 rotationRad = XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3 centerPosition = XMFLOAT3(0.f, 0.f, 0.f));
	void buildLightMatrix(XMFLOAT3 centerPosition = XMFLOAT3(0.f, 0.f, 0.f));
	void updateLight(Light directionalLight);

	// Render
	ID3D11ShaderResourceView* const* getShadowMapSRV();
	ID3D11ShaderResourceView* getShadowMapSRVNoneConst();
	ID3D11Buffer* const* getShadowMatrixConstantBuffer() const;
	void clearShadowMap();
	void bindInverseVpMatrixVS();
	void bindLightMatrixPS();
	void bindViewsAndRenderTarget();
};

#endif // !SHADOWMAPINSTANCE_H