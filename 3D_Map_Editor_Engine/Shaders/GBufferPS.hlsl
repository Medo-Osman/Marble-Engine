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
    float4 emissiveShadowMaskRT : SV_Target2;
    float4 ambientOcclusionRT : SV_Target3;
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

// Textures
Texture2D DiffuseTexture    : TEXTURE : register(t0);
Texture2D SpecularTexture   : TEXTURE : register(t1);
Texture2D NormalTexture     : TEXTURE : register(t2);

Texture2D ShadowMapTexture  : TEXTURE : register(t3);

// Samplers
SamplerState            sampState       : SAMPLER : register(s0);
SamplerComparisonState  shadowSampler   : SAMPLER : register(s1);

// Functions
float3 computeNormal(PS_IN input)
{
    float3 normalTex = normalize(NormalTexture.Sample(sampState, input.texCoord).xyz * 2 - 1);
    
    float3 normal = input.normal;
    float3 tangent = normalize(input.tangent);
    float3 bitangent = normalize(input.biTangent);
    float3x3 TBNMatrix = float3x3(tangent, bitangent, normal);
    
    return normalize(mul(normalTex, TBNMatrix));
}

float computeShadowFactor(PS_IN input)
{
    
    float2 shadowUV = input.shadowPosition.xy / input.shadowPosition.w * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;

    float shadowDepth = input.shadowPosition.z / input.shadowPosition.w;

    float shadowFactor = 0.f;
    [flatten]
    if (shadowDepth < 0.f || shadowDepth > 1.f) // if the pixels depth is beyond the shadow map, skip shadow sampling(Pixel is lit by light)
    {
        shadowFactor = 1.f;
    }
    else
    {
        const int sampleRange = 1;
        [unroll]
        for (int x = -sampleRange; x <= sampleRange; x++)
        {
            [unroll]
            for (int y = -sampleRange; y <= sampleRange; y++)
            {
                shadowFactor += ShadowMapTexture.SampleCmpLevelZero(shadowSampler, shadowUV, shadowDepth, int2(x, y));
            }
        }
        shadowFactor /= ((sampleRange * 2 + 1) * (sampleRange * 2 + 1));
    }
    
    return shadowFactor;
}

// PS Main
PS_OUT main(PS_IN input) : SV_TARGET
{
    // Albedo
    float3 albedo = matDiffuse.rgb;
    if (diffTextureExist)
        albedo *= DiffuseTexture.Sample(sampState, input.texCoord);
    
	// Normal
    float3 normal = normalize(input.normal.xyz);
    
    // Get Specular Value
    float3 specularColor = matSpecular.xyz;
    if (specTextureExist)
        specularColor *= SpecularTexture.Sample(sampState, input.texCoord);
    
    //Metallic
    float metallic = length(specularColor); // Phong to PBR Approximations, not correct at all
    
	//Rough
    float roughness = 1 / pow(pow(matShininess, 0.2), metallic); // Phong to PBR Approximations, not correct at all
    
    // Ambient Occlusionffreses
    float ambientOcclusion = 1.0;
    
    // Emissive
    float3 emissive = matEmissive.rgb;
    
    // Normal Map Calculation
    [flatten]
    if (normTextureExist)
        normal = computeNormal(input);
    
    // Shadow Mask
    float shadowFactor = computeShadowFactor(input);
    
    PS_OUT output;
    
    // Albedo, Metallic
    output.albedoMetallicRT = float4(albedo, metallic);
    
    // Normal, Roughness
    output.normalRoughnessRT = float4(normal, roughness);
    
    // Emissive, Ambient Occlusion
    output.emissiveShadowMaskRT = float4(emissive, shadowFactor);
    
    // Shadow Mask
    output.ambientOcclusionRT = float4(ambientOcclusion, ambientOcclusion, ambientOcclusion, 1.f);
    
    return output;
}