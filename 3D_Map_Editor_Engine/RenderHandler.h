#ifndef RENDERHANDLER_H
#define RENDERHANDLER_H

#include "Camera.h"
#include "Skybox.h"
#include "LightManager.h"
#include "ShadowMapInstance.h"
#include "ParticleSystem.h"
#include "ModelSelectionHandler.h"

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

    // Settings
    Settings* m_settings;

    // Render Target
    ComPtr< IDXGISwapChain > m_swapChain;
    ComPtr< ID3D11RenderTargetView > m_outputRTV;
    float clearColor[4] = { 0.1f, 0.11f, 0.13f, 1.f };

    // Viewport
    D3D11_VIEWPORT m_viewport;

    // Depth Buffer
    ComPtr< ID3D11DepthStencilView > m_depthStencilView;
    ComPtr< ID3D11Texture2D > m_depthStencilBuffer;
    ComPtr< ID3D11DepthStencilState > m_depthStencilState;
    ComPtr< ID3D11DepthStencilState > m_disabledDepthStencilState;

    // Render States
    ComPtr< ID3D11SamplerState > m_defaultSamplerState;
    ComPtr< ID3D11RasterizerState > m_defaultRasterizerState;
    ComPtr< ID3D11RasterizerState > m_wireframeRasterizerState;
    bool m_wireframeMode;

    // Shadow Mapping
    static const int SHADOW_MAP_SIZE = 3072; // 2048, 3072, 4096
    ShadowMapInstance m_shadowInstance;
    bool m_shadowMappingEnabled = true;

    // Blend State
    ComPtr< ID3D11BlendState > m_blendStateNoBlend;
    ComPtr< ID3D11BlendState > m_blendStateBlend;

    // Shader States
    std::vector<Shaders> m_shaderStates;

    // Render Objects
    using RenderObjectList = std::map<RenderObjectKey, RenderObject*, keyComp>;
    RenderObjectList m_renderObjects;
    RenderObjectList m_renderObjectsPBR;
    
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

    // Initialization Functions
    void initDeviceAndSwapChain();
    void initViewPort();
    void initDepthStencilBuffer();
    void initRenderStates();

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

    // Camera
    void updateCamera(XMVECTOR position, XMVECTOR rotation);

    // Render Objects
    RenderObjectKey newRenderObject(std::string modelName, ShaderStates shaderState = ShaderStates::PHONG);
    void setRenderObjectTextures(RenderObjectKey key, TexturePaths textures);
    void setRenderObjectTextures(RenderObjectKey key, TexturePathsPBR textures);
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

    // Lighting
    //void newLight(Light newLight);

    // Selection
    void updateSelectedObject(RenderObjectKey key, XMFLOAT3 newPosition);
    void deselectObject();
    XMFLOAT3 getRayWorldDirection(UINT pointX, UINT pointY);
    float selectionArrowPicking(UINT pointX, UINT pointY, char dimension);

    // Update
    void update(float dt);

    // Render
    void render();
};

#endif // !RENDERHANDLER_H