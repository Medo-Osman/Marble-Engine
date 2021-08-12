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

	// World Bounding Sphere
	BoundingSphere m_worldBoundingSphere;

	// Constant Buffers
	Buffer<VS_SHADOW_C_BUFFER> m_lightMatrixCBuffer;
	Buffer<XMMATRIX> m_shadowTextureMatrixCBuffer;

public:
	ShadowMapInstance();
	~ShadowMapInstance();

	// Initialize
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT width, UINT height);

	// Update
	void buildLightMatrix(Light directionalLight);

	// Render
	ID3D11ShaderResourceView* const* getShadowMapSRV();
	ID3D11Buffer* const* getShadowMatrixConstantBuffer() const;
	void bindViewsAndRenderTarget();
};

#endif // !SHADOWMAPINSTANCE_H