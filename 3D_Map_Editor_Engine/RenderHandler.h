#ifndef RENDERHANDLER_H
#define RENDERHANDLER_H

#include "Camera.h"
#include "Skybox.h"
#include "LightManager.h"
#include "ShadowMapInstance.h"
#include "ParticleSystem.h"
#include "ModelSelectionHandler.h"
#include "GBuffer.h"
#include "HBAOInstance.h"
#include "SSAOInstance.h"

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

    // Viewport
    D3D11_VIEWPORT m_viewport;

    // Depth Buffer
    ComPtr< ID3D11DepthStencilView > m_depthStencilView;
    ComPtr< ID3D11Texture2D > m_depthStencilBuffer;
    ComPtr< ID3D11DepthStencilState > m_depthStencilState;
    ComPtr< ID3D11DepthStencilState > m_disabledDepthStencilState;

    // Render States
    ComPtr< ID3D11SamplerState > m_defaultWrapSamplerState;
    ComPtr< ID3D11SamplerState > m_defaultBorderSamplerState;
    ComPtr< ID3D11RasterizerState > m_defaultRasterizerState;
    ComPtr< ID3D11RasterizerState > m_wireframeRasterizerState;
    bool m_wireframeMode;

    // Shadow Mapping
    static const int SHADOW_MAP_SIZE = 4096; // 2048, 3072, 4096
    ShadowMapInstance m_shadowInstance;
    bool m_shadowMappingEnabled = true;

    // Ambient Occlusion
    bool m_ssaoToggle = true;
    bool m_ssaoBlurToggle = true;
    float m_ssaoBlurSigma = 3.f;
    HBAOInstance m_HBAOInstance;
    SSAOInstance m_SSAOInstance;

    // Blur
    Shaders m_edgePreservingBlurCS;
    ComPtr< ID3D11ShaderResourceView > m_blurPingPongSRV;
    ComPtr< ID3D11UnorderedAccessView > m_blurPingPongUAV;
    std::unique_ptr< CS_BLUR_CBUFFER > m_blurConstantData;
    Buffer< CS_BLUR_CBUFFER > m_blurDirectionBuffer;

    // Down Sampling
    Shaders m_downsampleCS;
    RenderTexture m_halfResTexture;

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

    // - Buffers
    static const UINT NR_OF_BLOOM_BUFFERS = 3;
    static const UINT NR_OF_BLOOM_MIPS = 6;
    enum BloomBufferType { Base, FirstPingPong, SecondPingPong};

    RenderTexture m_bloomBuffers[NR_OF_BLOOM_BUFFERS];
    ComPtr< ID3D11UnorderedAccessView > m_bloomMipUAVs[NR_OF_BLOOM_BUFFERS][NR_OF_BLOOM_MIPS];

    // - Shaders
    Shaders m_bloomDownsampleShader;
    Shaders m_bloomUpsampleShader;

    // HDR Tonemapping
    Shaders m_tonemapShaders;
    PS_TONEMAP_CBUFFER m_tonemapConstantData;
    Buffer< PS_TONEMAP_CBUFFER > m_tonemapConstantBuffer;

    // Blend State
    ComPtr< ID3D11BlendState > m_blendStateNoBlend;
    ComPtr< ID3D11BlendState > m_blendStateBlend;

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
    ID3D11ShaderResourceView* m_shaderResourcesNullptr[5] = { m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr, m_shaderResourceNullptr };
    ID3D11UnorderedAccessView* m_unorderedAccessNullptr = nullptr;
    ID3D11UnorderedAccessView* m_unorderedAccessesNullptr[5] = { m_unorderedAccessNullptr, m_unorderedAccessNullptr, m_unorderedAccessNullptr, m_unorderedAccessNullptr, m_unorderedAccessNullptr };

    // Camera
    Camera m_camera;
    void initCamera();
    
    // Skybox
    Skybox m_skybox;

    // Lighting
    LightManager m_lightManager;

    // Timer
    Timer m_timer;

    // Particles
    ParticleSystem m_particleSystem;

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
    void initBloomPass(UINT width, UINT height);

    // Helper Functions
    void calculateBlurWeights(CS_BLUR_CBUFFER* bufferData, int radius, float sigma);

    // Pass Functions
    void lightPass();
    void downsamplePass();
    void blurSSAOPass();
    void bloomPass();

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
    int addLight(Light newLight, bool usedForShadowMapping = false);
    void removeLight(int id);
    void updateLight(Light* light, int id);
    void changeShadowMappingLight(Light* light, bool disableShadowCasting = false);
    
    // Render Modes
    bool* getWireframeModePtr();
    bool* getSsaoModePtr();
    bool* getSsaoBlurModePtr();

    // Lighting
    //void newLight(Light newLight);

    // Selection
    void updateSelectedObject(RenderObjectKey key, XMFLOAT3 newPosition);
    void deselectObject();
    XMFLOAT3 getRayWorldDirection(UINT pointX, UINT pointY);
    float selectionArrowPicking(UINT pointX, UINT pointY, char dimension);

    // Update
    void update(float dt);
    void updateShaderState(ShaderStates shaderState);
    void updatePassShaders();

    // Render
    void UIRenderShadowMap();
    void UIRenderPipelineTexturesWindow();
    void UITonemappingWindow();
    void UIssaoSettings();
    void UIbloomSettings();
    void UIEnviormentPanel();
    void render();
};

#endif // !RENDERHANDLER_H