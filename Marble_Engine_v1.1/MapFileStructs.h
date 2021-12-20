#ifndef MAPFILESTRUCTS_H
#define MAPFILESTRUCTS_H

#include <DirectXMath.h>
#include <string>
#include <vector>
#include "ShaderHelper.h"

// Material Data Structs
struct MaterialPhongData
{
	std::wstring diffusePath = L"";
	std::wstring specularPath = L"";
	std::wstring normalPath = L"";
	std::wstring displacementPath = L"";

	DirectX::XMFLOAT4	emissive = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	DirectX::XMFLOAT4	ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	DirectX::XMFLOAT4	diffuse = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	DirectX::XMFLOAT4	specular = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
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

// Mesh Data
struct MeshData
{
	std::string name;
	ShaderStates matType;
	MaterialPhongData matPhong;
	MaterialPBRData matPBR;
};

// Game Object Data
struct GameObjectData
{
	std::string modelFile = "";
	ShaderStates shaderType = ShaderStates::PHONG;
	DirectX::XMFLOAT3 scale = { 1.f,1.f,1.f };
	DirectX::XMFLOAT3 rotation = { 0.f,0.f,0.f };
	DirectX::XMFLOAT3 position = { 0.f,0.f,0.f };
	std::vector<MeshData> meshes;
};

// Map Prefixes

// - Game Object
const std::string MODEL_FILE_PREFIX = "mf";
const std::string SHADER_TYPE_PREFIX = "st";
const std::string TRANSFORM_SCALE_PREFIX = "ts";
const std::string TRANSFORM_ROTATION_PREFIX = "tr";
const std::string TRANSFORM_POSITION_PREFIX = "tp";

// - Mesh
const std::string NR_OF_MESHES_PREFIX = "nm";
const std::string MESH_NAME_PREFIX = "mn";

// - Material

// - - Shared
const std::string MAT_DIFF_PATH_PREFIX = "dp"; // Also Used as Albedo Tetxure Path
const std::string MAT_DIFFUSE_COL_PREFIX = "dc"; // Also Used as Albedo Color
const std::string MAT_NORM_PATH_PREFIX = "np";
const std::string MAT_DISP_PATH_PREFIX = "disp";

// - - Phong
const std::string PH_SPEC_PATH_PREFIX = "sp";
const std::string PH_EMISSIVE_COL_PREFIX = "ec";
const std::string PH_AMBIENT_COL_PREFIX = "ac";
const std::string PH_SPECULAR_COL_PREFIX = "sc";
const std::string PH_SHININESS_PREFIX = "ss";

const std::string PH_DIFF_TEX_PREFIX = "dt";
const std::string PH_SPEC_TEX_PREFIX = "st";
const std::string PH_NORM_TEX_PREFIX = "nt";

// - - PBR
const std::string PB_META_PATH_PREFIX = "mp";
const std::string PB_ROUG_PATH_PREFIX = "rp";
const std::string PB_EMIS_PATH_PREFIX = "ep";
const std::string PB_AMOC_PATH_PREFIX = "ap";

const std::string PB_METALLIC_PREFIX = "me";
const std::string PB_ROUGHNESS_PREFIX = "re";
const std::string PB_EMISS_STR_PREFIX = "es";
const std::string PB_TEXTURED_PREFIX = "te";
const std::string PB_EMI_TEXT_PREFIX = "et";

// Lights
const std::string NR_OF_LIGHTS_PREFIX = "ln";
const std::string LIGHT_POSITION_PREFIX = "lp";
const std::string LIGHT_DIRECTION_PREFIX = "ld";
const std::string LIGHT_INTENSITY_PREFIX = "li";
const std::string LIGHT_COLOR_PREFIX = "lc";
const std::string LIGHT_SPOTANGLES_PREFIX = "ls";
const std::string LIGHT_RANGE_PREFIX = "lr";
const std::string LIGHT_TYPE_PREFIX = "lt";
const std::string LIGHT_ENABLED_PREFIX = "le";
const std::string LIGHT_ISCASTINFSHADOW_PREFIX = "lsh";
const std::string LIGHT_ROTATION_PREFIX = "lrot";

#endif //!MAPFILESTRUCTS_H