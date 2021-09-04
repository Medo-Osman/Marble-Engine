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
    XMFLOAT4    color;
    float       spotAngle;
    XMFLOAT3    attenuation;
    float       range;
    int         type;
    BOOL        enabled;
    BOOL        isCastingShadow;
};

const UINT LIGHT_CAP = 20;

struct PS_LIGHT_BUFFER
{
    Light lights[LIGHT_CAP];
    UINT nrOfLights;
    XMFLOAT3 pad;
};

struct PS_COLOR_ANIMATION_BUFFER
{
    float colorOpacity = 1.f;
    XMFLOAT3 pad;
};

struct GS_PARTICLE_CBUFFER
{
    XMMATRIX viewProjectionMatrix;
    XMFLOAT3 camPosition;
    float gameTime;
    XMFLOAT3 emitPosition;
    float deltaTime;
    XMFLOAT3 emitDirection;
    float pad;
};