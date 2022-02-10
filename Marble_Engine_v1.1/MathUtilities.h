#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include <stdlib.h>
#include <ctime>
#include <DirectXMath.h>
using namespace DirectX;

static float frand(float low, float high)
{
	return low + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX / (high - low)));
}

static float lerpF(float from, float to, float weight)
{
	return from + weight * (to - from);
}

static float smoothstep(float edge0, float edge1, float x)
{
    // https://en.wikipedia.org/wiki/Smoothstep
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3 - 2 * x);
}

static float remap(float value, XMFLOAT2 inMinMax, XMFLOAT2 outMinMax)
{
    return outMinMax.x + (value - inMinMax.x) * (outMinMax.y - outMinMax.x) / (inMinMax.y - inMinMax.x);
}

static XMFLOAT3 normalizeRotationRad(XMFLOAT3 rotation)
{
    return XMFLOAT3(std::fmod(rotation.x, XM_2PI), fmod(rotation.y, XM_2PI), fmod(rotation.z, XM_2PI));
}

static XMMATRIX XM_CALLCONV lookAtMatrix(FXMVECTOR Position, FXMVECTOR Direction, FXMVECTOR Up)
{
    XMVECTOR dir = Direction;
    if (XMVector3Equal(dir, XMVectorZero()))
        dir = XMVectorSet(0.f, 0.1f, 0.f, 0.f);

    assert(!XMVector3IsInfinite(dir));
    assert(!XMVector3Equal(Up, XMVectorZero()));
    assert(!XMVector3IsInfinite(Up));

    XMVECTOR R2 = XMVector3Normalize(dir);

    XMVECTOR R0 = XMVector3Cross(Up, R2);
    R0 = XMVector3Normalize(R0);

    XMVECTOR R1 = XMVector3Cross(R2, R0);

    XMMATRIX matrix(R0, R1, R2, Position);

    return matrix;
}

#endif // !MATHUTILITIES_H