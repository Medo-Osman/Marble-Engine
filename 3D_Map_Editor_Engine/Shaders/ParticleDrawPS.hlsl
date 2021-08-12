// Vertex
struct PS_IN
{
    float4 position : SV_Position;
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD;
};

// Textures
Texture2DArray textureArray : TEXTURE : register(t0);

// Samplers
SamplerState sampState      : SAMPLER : register(s0);

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    return textureArray.Sample(sampState, float3(input.texCoord, 0)) * input.color;
}