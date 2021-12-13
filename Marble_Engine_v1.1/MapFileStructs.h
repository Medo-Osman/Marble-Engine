#ifndef MAPFILESTRUCTS_H
#define MAPFILESTRUCTS_H

#include <DirectXMath.h>
#include "ShaderHelper.h"

struct MaterialPhongData
{
	std::wstring diffusePath = L"";
	std::wstring specularPath = L"";
	std::wstring normalPath = L"";
	std::wstring displacementPath = L"";

	XMFLOAT4	emissive = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	XMFLOAT4	ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	XMFLOAT4	diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	XMFLOAT4	specular = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	float		shininess = 30.f;

	BOOL diffTextureExists = false;
	BOOL specTextureExists = false;
	BOOL normTextureExists = false;
};

struct MaterialPBRData
{
	std::wstring albedoPath = L"";
	std::wstring normalPath = L"";
	std::wstring metallicPath = L"";
	std::wstring roughnessPath = L"";
	std::wstring emissivePath = L"";
	std::wstring ambientOcclusionPath = L"";
	std::wstring displacementPath = L"";

	DirectX::XMFLOAT3 albedo = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	float metallic = 0.f;
	float roughness = 0.f;
	float emissiveStrength = 0.f;
	bool materialTextured = true;
	bool emissiveTextured = false;
};

struct MeshData
{
	std::string name;
	ShaderStates matType;
	MaterialPhongData matPhong;
	MaterialPBRData matPBR;
};

struct GameObjectData
{
	std::string modelFile = "";
	ShaderStates shaderType = ShaderStates::PHONG;
	XMFLOAT3 scale = { 1.f,1.f,1.f };
	XMFLOAT3 rotation = { 0.f,0.f,0.f };
	XMFLOAT3 position = { 0.f,0.f,0.f };
	std::vector<MeshData> meshes;
};

#endif //!MAPFILESTRUCTS_H