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
    float3 matAlbedo;
    float matMetallic;
    float matRoughness;
    float matEmissiveStrength;
    int materialTextured;
    int emissiveTextured;
};

// Textures
Texture2D AlbedoTexture             : register(t0);
Texture2D NormalTexture             : register(t1);
Texture2D MetallicTexture           : register(t2);
Texture2D RoughnessTexture          : register(t3);
Texture2D EmissiveTexture           : register(t4);
Texture2D AmbientOcclusionTexture   : register(t5);

Texture2D ShadowMapTexture          : register(t6);

// Samplers
SamplerState            sampState       : register(s1); // Imgui uses slot 0, use 1 for default
SamplerComparisonState  shadowSampler   : register(s2);

// Functions
float3 computeNormal(PS_IN input)
{
    float3 normalTex = normalize(NormalTexture.Sample(sampState, input.texCoord).xyz * 2 - 1);
    //normalTex.z *= -1;
    //normalTex.y *= -1;
    
    float3 normal = input.normal;
    float3 tangent = normalize(input.tangent - dot(input.tangent, normal) * normal);
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
PS_OUT main(PS_IN input)
{
    // Albedo
    float3 albedo = matAlbedo;
	// Normal
    float3 normal = normalize(input.normal.xyz);
    //Metallic
    float metallic = matMetallic;
	//Rough
    float roughness = matRoughness;
    // Ambient Occlusion
    float ambientOcclusion = 1.0;
    // Emissive
    float3 emissive;
    
    // Normal Map Calculation
    [flatten]
    if (materialTextured)
    {
        float4 albedoTex = AlbedoTexture.Sample(sampState, input.texCoord);
        clip(albedoTex.a < 0.1f ? -1 : 1);
        albedo *= albedoTex.rgb;
        normal = computeNormal(input);
        roughness = RoughnessTexture.Sample(sampState, input.texCoord).g;
        metallic = MetallicTexture.Sample(sampState, input.texCoord).b;
        ambientOcclusion = AmbientOcclusionTexture.Sample(sampState, input.texCoord).r;
    }
    
    [flatten]
    if (emissiveTextured)
        emissive = EmissiveTexture.Sample(sampState, input.texCoord).rgb * matEmissiveStrength;
    else
        emissive = albedo * matEmissiveStrength;
    
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