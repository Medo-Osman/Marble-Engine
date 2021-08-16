// Globals
#define LIGHT_CAP 20

#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2

// Vertex
struct PS_IN
{
    float4 position         : SV_POSITION;
    float3 wPosition        : POSITION;
    float4 shadowPosition   : POSITION1;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BITANGENT;
    float2 texCoord         : TEXCOORD;
};

struct PS_OUT
{
    float4 albedoMetallicRT : SV_Target0;
    float4 normalRoughnessRT : SV_Target1;
    float4 emissiveAmbientOcclusionRT : SV_Target2;
};

// Constant Buffers
cbuffer materialBuffer : register(b0)
{
    float4  matEmissive;
    float4  matAmbient;
    float4  matDiffuse;
    float4  matSpecular;
    float   matShininess;
    
    bool    diffTextureExist;
    bool    specTextureExist;
    bool    normTextureExist;
};

struct Light
{
    float4  position;
    float4  direction;
    float4  color;
    float   spotAngle;
    float3  attenuation;
    float   range;
    int     type;
    bool    enabled;
    float   pad; // Light Array needs correct byte offset
};

cbuffer lightBuffer : register(b1)
{
    Light lights[LIGHT_CAP];
    uint nrOfLights;
};

struct ColorValues
{
    float4 diffuse;
    float4 specular;
};

cbuffer cameraBuffer : register(b2)
{
    float4 camPosition;
};

cbuffer shadowBuffer : register(b3)
{
    matrix shadowTextureMatrix;
};

// Textures
Texture2D diffuseTexture    : TEXTURE : register(t0);
Texture2D specularTexture   : TEXTURE : register(t1);
Texture2D normalTexture     : TEXTURE : register(t2);

Texture2D shadowMap         : TEXTURE : register(t3);

// Samplers
SamplerState sampState                  : SAMPLER : register(s0);
SamplerComparisonState shadowSampler    : SAMPLER : register(s1);

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    float4 emissiveColor = matEmissive;
    float4 ambientColor = matAmbient;
    float4 diffuseColor = 0;
    float4 specularColor = 0;
    
    // Albedo
    float3 albedo = matDiffuse;
	// Normal
    float3 normal = normalize(input.NormalWS.xyz);
    //Metallic
    float metallic = MaterialCB.metallic;
	//Rough
    float roughness = MaterialCB.roughness;
    // Ambient Occlusion
    float ambientOcclusion = 1.0;
    // Emissive
    float3 emissive = albedo * MaterialCB.emissiveStrength;
    
    // Normal Map Calculation
    [flatten]
    if (normTextureExist)
    {
        float3 normalTex = normalTexture.Sample(sampState, input.texCoord).xyz;
        normalTex = normalize(normalTex);
        normalTex.x = normalTex.x * 2.f - 1.f;
        normalTex.y = -normalTex.y * 2.f + 1.f;
        normalTex.z = normalTex.z;
        
        float3x3 TBNMatrix = float3x3(input.tangent, input.biTangent, input.normal);
        input.normal = normalize(mul(normalTex, TBNMatrix));
    }
    
    PS_OUT output;
    
    // Albedo, Metallic
    output.albedoMetallicRT = float4(albedo.rgb, metallic);
    
    // Normal, Roughness
    output.normalRoughnessRT = float4(normal.rgb, roughness);
    
    // Emissive, Ambient Occlusion
    output.emissiveAmbientOcclusionRT = float4(emissive.rgb, ambientOcclusion);
    
    return output;
}