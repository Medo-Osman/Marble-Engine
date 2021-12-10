#ifndef RENDERHANDLER_H
#define RENDERHANDLER_H

#include "Camera.h"
#include "Sky.h"
#include "LightManager.h"
#include "ShadowMapInstance.h"
#include "ParticleSystem.h"
#include "ModelSelectionHandler.h"
#include "GBuffer.h"
#include "SSAOInstance.h"
#include "HBAOInstance.h"

struct Settings
{
    int width;
    int height;
    float fov;
    float mouseSensitivity;
};

class RenderObjectKey
{
private:
    int key;
    bool valid;
    ShaderStates objectType;
    friend class RenderHandler;
    friend struct keyComp;
public:
    RenderObjectKey() { key = -1; valid = false; objectType = ShaderStates::PHONG; }
    bool isValid() { return valid; }
};

struct keyComp
{
    bool operator()(const RenderObjectKey& left, const RenderObjectKey& right) const
    {
        return (left.key < right.key);
    }
};

class RenderHandler
{
private:
    RenderHandler();
    ~RenderHandler();

    // Device
    ComPtr< ID3D11Device > m_device;
    ComPtr< ID3D11DeviceContext > m_deviceContext;
    
    // Window
    HWND* m_window;
    UINT m_clientWidth;
    UINT m_clientHeight;

    // Settings
    Settings* m_settings;

    // Render Targets
    ComPtr< IDXGISwapChain > m_swapChain;
    GBuffer m_gBuffer;
    RenderTexture m_hdrRTV;
    ComPtr< ID3D11RenderTargetView > m_outputRTV;
    float clearColor[4] = { 0.1f, 0.11f, 0.13f, 1.f };
    float clearColorBlack[4] = { 0.f, 0.f, 0.f, 1.f };
    float clearColorWhite[4] = { 1.f, 1.f, 1.f, 1.f };

    UINT clearBlackUint[1] = { 1 };

    // Viewport
    D3D11_VIEWPORT m_viewport;

    // Depth Buffer
    ComPtr< ID3D11DepthStencilView > m_depthStencilView;
    ComPtr< ID3D11Texture2D > m_depthStencilBuffer;
    ComPtr< ID3D11DepthStencilState > m_depthStencilState;
    ComPtr< ID3D11DepthStencilState > m_readOnlyDepthStencilState;
    ComPtr< ID3D11DepthStencilState > m_disabledDepthStencilState;

    // Render States
    ComPtr< ID3D11SamplerState > m_defaultWrapSamplerState;
    ComPtr< ID3D11SamplerState > m_defaultBorderSamplerState;
    ComPtr< ID3D11SamplerState > m_whiteBorderSamplerState;
    ComPtr< ID3D11SamplerState > m_blackBorderComparisonSamplerState;

    ComPtr< ID3D11RasterizerState > m_defaultRasterizerState;
    ComPtr< ID3D11RasterizerState > m_wireframeRasterizerState;
    ComPtr< ID3D11RasterizerState > m_cullOffRasterizerState;
    ComPtr< ID3D11RasterizerState > m_cullOffWireframeRasterizerState;
    bool m_wireframeMode;

    // Shadow Mapping
    static const int SHADOW_MAP_SIZE = 4096; // 2048, 3072, 4096
    ShadowMapInstance m_shadowInstance;
    bool m_shadowMappingEnabled = true;

    // Ambient Occlusion
    bool m_ssaoToggle = true;
    bool m_useHBAOToggle = false;
    bool m_ssaoBlurToggle = true;
    float m_ssaoBlurSigma = 3.f;
    HBAOInstance m_HBAOInstance;
    SSAOInstance m_SSAOInstance;

    // Blur
    Shaders m_edgePreservingBlurCS;
    ComPtr< ID3D11ShaderResourceView > m_blurPingPongSRV;
    ComPtr< ID3D11UnorderedAccessView > m_blurPingPongUAV;
    std::unique_ptr< CS_BLUR_CBUFFER > m_blurCData;
    Buffer< CS_BLUR_CBUFFER > m_blurCBuffer;

    // Down Sampling
    Shaders m_downsampleCS;
    RenderTexture m_halfResTexture;

    // Volumetric Sun Scattering
    bool m_volumetricSunToggle = true;
    bool m_volumetricSunWireframeToggle = false;
    std::unique_ptr<Mesh<VertexPosNormTexTan>> m_lightVolumeMesh;
    Shaders m_volumetricSunShaders;
    RenderTexture m_volumetricAccumulationRTV;

    Buffer< VS_WVP_CBUFFER > m_lightVolumeWvpCBuffer;
    Buffer< DS_TESSELLATION_CBUFFER > m_lightVolumeTessCBuffer;
    
    // Fog
    bool m_fogToggle = true;

    // Bloom
    // - Pass
    bool m_bloomToggle = true;
    float m_bloomThreshold = 0.3f;
    float m_bloomKnee = 0.1f;

    // - Constant Buffers
    std::unique_ptr< CS_DOWNSAMPLE_CBUFFER > m_bloomDownsampleData;
    Buffer< CS_DOWNSAMPLE_CBUFFER > m_bloomDownsampleBuffer;
    std::unique_ptr< CS_UPSAMPLE_CBUFFER > m_bloomUpsampleData;
    Buffer< CS_UPSAMPLE_CBUFFER > m_bloomUpsampleBuffer;

    // - Texture Buffers
    static const UINT NR_OF_BLOOM_BUFFERS = 3;
    static const UINT NR_OF_BLOOM_MIPS = 6;
    enum BloomBufferType { Base, FirstPingPong, SecondPingPong};

    RenderTexture m_bloomBuffers[NR_OF_BLOOM_BUFFERS];
    ComPtr< ID3D11UnorderedAccessView > m_bloomMipUAVs[NR_OF_BLOOM_BUFFERS][NR_OF_BLOOM_MIPS];

    // - Shaders
    Shaders m_bloomDownsampleShader;
    Shaders m_bloomUpsampleShader;

    // Lens Flare
    bool m_lensFlareToggle = true;
    Shaders m_lensFlareShaders;
    LENS_FLARE_CBUFFER m_lensFlareCData;
    Buffer< LENS_FLARE_CBUFFER > m_lensFlareCBuffer;
    ID3D11ShaderResourceView* m_lensFlareTexturesSRV;

    // Adaptive Exposure
    bool m_adaptiveExposureToggle = true;
    Shaders m_adaptiveExposureHistogramShaders;
    Shaders m_adaptiveExposureAveragingShaders;

    PS_HISTOGRAM_CBUFFER m_histogramCData;
    Buffer< PS_HISTOGRAM_CBUFFER > m_histogramCBuffer;
    PS_HISTOGRAM_AVERAGING_CBUFFER m_histogramAveragingCData;
    Buffer< PS_HISTOGRAM_AVERAGING_CBUFFER > m_histogramAveragingCBuffer;

    ComPtr< ID3D11UnorderedAccessView > m_histogramUAV;
    ComPtr< ID3D11UnorderedAccessView > m_luminanceUAV;
    ComPtr< ID3D11ShaderResourceView > m_luminanceSRV;

    // HDR Tonemapping
    Shaders m_tonemapShaders;
    PS_TONEMAP_CBUFFER m_tonemapCData;
    Buffer< PS_TONEMAP_CBUFFER > m_tonemapCBuffer;

    // Blend State
    ComPtr< ID3D11BlendState > m_blendStateNoBlend;
    ComPtr< ID3D11BlendState > m_blendStateBlend;
    ComPtr< ID3D11BlendState > m_blendStateAdditiveBlend;
    ComPtr< ID3D11BlendState > m_blendStatePreMultipliedAlphaBlend;

    // Shader States
    std::vector<Shaders> m_shaderStates;
    Shaders m_lightPassShaders;

    // Render Objects
    using RenderObjectList = std::map<RenderObjectKey, RenderObject*, keyComp>;
    RenderObjectList m_renderObjects;
    RenderObjectList m_renderObjectsPBR;

    // Null Pointer Views
    ID3D11RenderTargetView* m_renderTargetNullptr = nullptr;
    ID3D11ShaderResourceView* m_shaderResourceNullptr = nullptr;
    ID3D11ShaderResourceView* m_shaderResourcesNullptr[7] = { m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr };
    ID3D11UnorderedAccessView* m_unorderedAccessNullptr = nullptr;
    ID3D11UnorderedAccessView* m_unorderedAccessesNullptr[5] = { m_unorderedAccessNullptr, m_unorderedAccessNullptr, m_unorderedAccessNullptr, m_unorderedAccessNullptr, m_unorderedAccessNullptr };

    // Camera
    Camera m_camera;
    void initCamera();
    
    // Skybox
    Sky m_sky;

    // Lighting
    LightManager m_lightManager;

    // Timer
    Timer m_timer;

    // Particles
    std::map<std::string, ParticleSystem> m_particleSystems;

    // Model Selection
    ModelSelectionHandler m_modelSelectionHandler;
    RenderObjectKey m_selectedObjectKey;
    Shaders m_selectionShaders;
    float m_animationDirection;
    PS_COLOR_ANIMATION_BUFFER m_selectionAnimationData;
    Buffer<PS_COLOR_ANIMATION_BUFFER> m_selectionCBuffer;

    // UI
    ImGui::FileBrowser m_fileDialog;
    WCHAR tempName[64];
    CubemapType m_loadNewCubemapType = CubemapType::None;

    // Initialization Functions
    void initDeviceAndSwapChain();
    void initRenderTarget(RenderTexture& rtv, UINT width, UINT height, UINT mipLevels = 1);
    void initRenderTargets();
    void initViewPort();
    void initDepthStencilBuffer();
    void initRenderStates();
    void initSSAOBlurPass(UINT width, UINT height, DXGI_FORMAT format);
    void initVolumetricSunPass();
    void initBloomPass(UINT width, UINT height);
    void initAdaptiveExposurePass();

    // Helper Functions
    void calculateBlurWeights(CS_BLUR_CBUFFER* bufferData, int radius, float sigma);

    // Pass Functions
    void lightPass();
    void downsamplePass();
    void blurSSAOPass();
    void volumetricSunPass();
    void bloomPass();
    void adaptiveExposurePass(float deltaTime);
    void particlePass();

public:
    RenderHandler(RenderHandler const&) = delete;
    void operator=(RenderHandler const&) = delete;

    static RenderHandler* getInstance()
    {
        static RenderHandler instance;
        return &instance;
    }

    // Initialization
    void initialize(HWND* window, Settings* settings);

    // Client Dimensions
    UINT getClientWidth() const;
    UINT getClientHeight() const;

    // Camera
    void updateCamera(XMVECTOR position, XMVECTOR rotation);

    // Render Objects
    RenderObjectKey newRenderObject(std::string modelName, ShaderStates shaderState = ShaderStates::PHONG);
    void setRenderObjectTextures(RenderObjectKey key, TexturePaths textures);
    void setRenderObjectTextures(RenderObjectKey key, TexturePathsPBR textures);
    void setRenderObjectMaterial(RenderObjectKey key, PS_MATERIAL_BUFFER material);
    void setRenderObjectMaterialPBR(RenderObjectKey key, PS_MATERIAL_PBR_BUFFER material);
    void updateRenderObjectWorld(RenderObjectKey key, XMMATRIX worldMatrix);
    void deleteRenderObject(RenderObjectKey key);
    RenderObjectKey setShaderState(RenderObjectKey key, ShaderStates shaderState);
    void modelTextureUIUpdate(RenderObjectKey key);

    // Lights
    int addLight(Light newLight, XMFLOAT3 rotationRad = XMFLOAT3(0, 0, 0), bool usedForShadowMapping = false);
    void removeLight(int id);
    void updateLight(Light* light, int id);
    void changeShadowMappingLight(Light* light, XMFLOAT3 rotationRad = XMFLOAT3(0,0,0), bool disableShadowCasting = false);
    
    // Render Modes
    bool* getWireframeModePtr();
    bool* getSsaoModePtr();
    bool* getSsaoBlurModePtr();

    // Selection
    void updateSelectedObject(RenderObjectKey key, XMFLOAT3 newPosition);
    void deselectObject();
    XMFLOAT3 getRayWorldDirection(UINT pointX, UINT pointY);
    float selectionArrowPicking(UINT pointX, UINT pointY, char dimension);

    // Update
    void update(double dt);
    void resetParticles();
    void updateShaderState(ShaderStates shaderState);
    void updatePassShaders();

    // UI
    void UIRenderShadowMap();
    void UIRenderPipelineTexturesWindow();
    void UITonemappingWindow();
    void UIssaoSettings();
    void UIadaptiveExposureSettings();
    void UIVolumetricSunSettings();
    void UIbloomSettings();
    void UILensFlareSettings();
    void UIEnviormentPanel();

    // Render
    void render(double dt);
};

#endif // !RENDERHANDLER_H