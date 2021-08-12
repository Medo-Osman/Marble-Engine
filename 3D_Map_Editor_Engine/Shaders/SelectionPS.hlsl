// Vertex
struct PS_IN
{
    float4 position     : SV_POSITION;
};

// Constant Buffers
cbuffer colorAnimation : register(b4)
{
    float colorOpacity; // For fade in out animation
    float3 pad;
};

// Samplers
SamplerState sampState  : SAMPLER : register(s0);

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    return float4(0.2f, 0.53f, 1.f, colorOpacity);
    //return float4(colorOpacity, colorOpacity, colorOpacity, 1.f);
}