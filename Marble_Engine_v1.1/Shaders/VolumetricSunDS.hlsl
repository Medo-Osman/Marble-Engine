// Globals
#define NUM_CONTROL_POINTS 4

// Vertex In
struct DS_IN
{
    float3 wPosition : POSITION;
    float2 texCoord  : TEXCOORD;
};

// Vertex Out
struct DS_OUT
{
    float4 position  : SV_POSITION;
    float3 wPosition : POSITION;
    float2 texCoord  : TEXCOORD;
};

// Constant Buffers
cbuffer WVPBuffer : register(b0)
{
    matrix vpMatrix;
    matrix worldMatrix;
    matrix normalMatrix;
};

cbuffer lightSpaceMatrices : register(b1)
{
    matrix invLightViewMatrix;
    matrix invLightProjectionMatrix;
};

cbuffer sunData : register(b2)
{
    float3 sunDirection;
    float sunBoundsDepth;
    float3 sunColor;
};

// Tesselation Factor
struct HullShaderConstantDataOutput
{
    float EdgeTessFactor[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

// Texture
Texture2D ShadowMap : TEXTURE : register(t0);
SamplerState sampState : SAMPLER : register(s0);

// Main
[domain("quad")]
DS_OUT main(HullShaderConstantDataOutput input, float2 domain : SV_DomainLocation, const OutputPatch<DS_IN, NUM_CONTROL_POINTS> patch)
{
    DS_OUT output;

    // Position
    output.wPosition = lerp(
                            lerp(patch[0].wPosition, patch[1].wPosition, domain.x),
                            lerp(patch[2].wPosition, patch[3].wPosition, domain.x),
                            domain.y);
    
    // Texcord
    output.texCoord = lerp(
                            lerp(patch[0].texCoord, patch[1].texCoord, domain.x),
                            lerp(patch[2].texCoord, patch[3].texCoord, domain.x),
                            domain.y);
    
    // Dispalcement
    if (output.texCoord.x != 0.f && output.texCoord.y != 0.f && output.texCoord.x != 1.f && output.texCoord.y != 1.f)
    {
        float fDisplacement = ShadowMap.SampleLevel(sampState, output.texCoord, 0).r - 0.0005f; // Shadow Map (0 - 1)
        output.wPosition = output.wPosition + (sunDirection * (fDisplacement * sunBoundsDepth));
    }
    
    output.position = mul(float4(output.wPosition, 1.f), vpMatrix);
    
    return output;
}