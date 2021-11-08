#include "pch.h"

// Byte width must be multiple of 16

struct VS_WVP_CBUFFER
{
    XMMATRIX wvp;
    XMMATRIX worldMatrix;
    XMMATRIX normalMatrix;
    VS_WVP_CBUFFER()
    {
        wvp = XMMatrixIdentity();
        worldMatrix = XMMatrixIdentity();
        normalMatrix = XMMatrixIdentity();
    }
};

struct VS_SKYBOX_MATRIX_CBUFFER
{
    XMMATRIX vpMatrix;
    VS_SKYBOX_MATRIX_CBUFFER()
    {
        vpMatrix = XMMatrixIdentity();
    }
};

struct Light
{
    XMFLOAT4	position;
    XMFLOAT4    direction;
    XMFLOAT3    color;
    float       intensity = 1.f;
    float       spotAngle;
    XMFLOAT3    attenuation;
    float       range;
    int         type;
    BOOL        enabled;
    BOOL        isCastingShadow;
};

struct LightHelper
{
    XMFLOAT3 rotationDeg;
};

const UINT LIGHT_CAP = 40;

struct PS_LIGHT_BUFFER
{
    Light lights[LIGHT_CAP];
    UINT nrOfLights;
    float enviormentDiffContribution = 0.1f;
    float enviormentSpecContribution = 0.f;
    BOOL volumetricSunScattering = TRUE;//FALSE;
    BOOL fog = TRUE;//FALSE;
    XMFLOAT3 pad;
};

struct PS_COLOR_ANIMATION_BUFFER
{
    float colorOpacity = 1.f;
    XMFLOAT3 pad;
};

struct PARTICLE_CBUFFER
{
    XMMATRIX viewMatrix;
    XMMATRIX projMatrix;
    XMFLOAT3 camPosition;
    float gameTime;
    XMFLOAT3 emitPosition = XMFLOAT3(0.f, 2.f, 0.f);
    float deltaTime;
    float maxParticles;
    XMFLOAT3 pad;
};

struct PARTICLE_STYLE
{
    XMFLOAT3 colorBegin = XMFLOAT3(1.f, 1.f, 1.f);
    float colorBias = 0.2f;
    XMFLOAT3 colorEnd = XMFLOAT3(.9f, .9f, .9f);
    float colorIntensity = 1.f;
    float scaleVariationMax = 0.2f;
    float rotationVariationMax = XM_PIDIV4;
    float lifetime = 1.f;
    BOOL useNoise = true;
    XMFLOAT3 emitDirection = XMFLOAT3(0.f, 1.f, 0.f);
    float emitInterval = 0.1f;
    BOOL randomizePosition = false;
    XMFLOAT3 randomizePosBounds = XMFLOAT3(0.f, 0.f, 0.f);
    BOOL randomizeDirection = false;
    BOOL dieOnCollition = false;
    BOOL fadeInAndOut = false;
    UINT idInterval = 0; // Used to reset particle ID range
};

const int MAX_BLUR_RADIUS = 15;
struct CS_BLUR_CBUFFER
{
    XMMATRIX projectionMatrix;
    int radius;
    BOOL direction;
    XMFLOAT2 pad;
    alignas(16) float weights[MAX_BLUR_RADIUS]; // BLUR_RADIUS + 1 * 4 bytes
};

struct CS_DOWNSAMPLE_CBUFFER
{
    XMFLOAT4 threshold; // x = threshold, y = threshold - knee, z = knee * 2, w = 0.25 / knee
    XMFLOAT2 mipDimensions;
    float mipLevel;
    float exposure = 0.f;
};

struct CS_UPSAMPLE_CBUFFER
{
    XMFLOAT2 mipDimensions;
    float mipLevel;
    float pad;
};

struct PS_HISTOGRAM_CBUFFER
{
    UINT texWidth;
    UINT texHeight;
    float minLogLuminance = -2.f; // min
    float oneOverLogLuminanceRange = 1.f / (2.f + 20.f); // 1 / (abs min + range)
};

struct PS_HISTOGRAM_AVERAGING_CBUFFER
{
    UINT pixelCount;
    float minLogLuminance = -1.5f; // min
    float logLuminanceRange = 20.f; // range
    float deltaTime;
    float tau = 0.3f;
    XMFLOAT3 pad;
};

struct PS_TONEMAP_CBUFFER
{
    // Should be expressed as a relative expsure value (-2, -1, 0, +1, +2 )
    float Exposure = 0.0f;

    // Gamma, 2.2 by default
    float Gamma = 2.2f;

    // ACES Filmic
    // See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
    float ACESss = 0.5f; // Shoulder strength
    float ACESls = 0.1f; // Linear strength
    float ACESla = 0.01f; // Linear angle
    float ACESts = 0.55f; // Toe strength
    float ACEStn = 0.02f; // Toe Numerator
    float ACEStd = 0.3f; // Toe denominator

    // Note ACEStn/ACEStd = Toe angle.
    float LinearWhite = 12.0f;

    // Padding
    XMFLOAT3 pad;
};

struct DS_TESSELLATION_CBUFFER
{
    float tessFactor;
    XMFLOAT3 pad;
};

struct DS_SUN_DATA_CBUFFER
{
    XMFLOAT3 sunDirection;
    float sunIntensity;
    XMFLOAT3 sunColor;
    float pad;
};