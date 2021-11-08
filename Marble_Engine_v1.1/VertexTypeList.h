#ifndef VERTEXTYPELIST_H
#define VERTEXTYPELIST_H

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

enum class LayoutType { POS_NOR_TEX_TAN, POS_NOR_TEX, POS_TEX_FINDEX, POS_TEX, POS_COL, POS, PARTICLE, NONE };


struct VertexPos
{
	XMFLOAT3 position;
};
static const unsigned int VertexPosElementCount = 1;

const D3D11_INPUT_ELEMENT_DESC VertexPosDesc[] =
{
	{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

struct VertexPosTexFrustumIndex
{
	XMFLOAT3 position;
	XMFLOAT2 texCoord;
	UINT frustumIndex;
};
static const unsigned int VertexPosTexFrustumIndexElementCount = 3;

const D3D11_INPUT_ELEMENT_DESC VertexPosTexFrustumIndexDesc[] =
{
	{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "FRUSTUM_INDEX",  0, DXGI_FORMAT_R32_UINT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


struct VertexPosCol
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};
static const unsigned int VertexPosColElementCount = 2;

const D3D11_INPUT_ELEMENT_DESC VertexPosColDesc[] =
{
	{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


struct VertexPosTex
{
	XMFLOAT3 position;
	XMFLOAT2 texCoord;
};
static const unsigned int VertexPosTexElementCount = 2;

const D3D11_INPUT_ELEMENT_DESC VertexPosTexDesc[] =
{
	{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


struct VertexPosNormTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texCoord;
};
static const unsigned int VertexPosNormTexElementCount = 3;

const D3D11_INPUT_ELEMENT_DESC VertexPosNormTexDesc[] =
{
	{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


struct VertexPosNormTexTan
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 bitangent;
	XMFLOAT2 texCoord;
};
static const unsigned int VertexPosNormTexTanElementCount = 5;

const D3D11_INPUT_ELEMENT_DESC VertexPosNormTexTanDesc[] =
{
	{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BITANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,		0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

struct VertexParticle
{
	XMFLOAT3 position;
	XMFLOAT3 velocity;
	XMFLOAT2 size;
	float rotation; // radiance
	float age;
	UINT type;
	UINT maxId;
};
static const unsigned int VertexParticleElementCount = 7;

const D3D11_INPUT_ELEMENT_DESC VertexParticleDesc[] =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "VELOCITY",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SIZE",		0, DXGI_FORMAT_R32G32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "ROTATION",	0, DXGI_FORMAT_R32_FLOAT,			0, 32,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "AGE",		0, DXGI_FORMAT_R32_FLOAT,			0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TYPE",		0, DXGI_FORMAT_R32_UINT,			0, 40,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "MAXID",		0, DXGI_FORMAT_R32_UINT,			0, 44,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

const D3D11_SO_DECLARATION_ENTRY VertexParticleSoDecl[] =
{
	{ 0, "POSITION", 0, 0, 3, 0 },
	{ 0, "VELOCITY", 0, 0, 3, 0 },
	{ 0, "SIZE", 0, 0, 2, 0 },
	{ 0, "ROTATION", 0, 0, 1, 0 },
	{ 0, "AGE", 0, 0, 1, 0 },
	{ 0, "TYPE", 0, 0, 1, 0 },
	{ 0, "MAXID", 0, 0, 1, 0 },
};

#endif // !VERTEXTYPELIST_H