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