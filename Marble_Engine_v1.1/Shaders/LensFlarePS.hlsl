// Globals
static const float TEX_STRENGTH = 0.1f; // 0 - 1

// Vertex
struct PS_IN
{
    float4 Position                 : SV_POSITION;
    float3 TexCoord                 : TEXCOORD0; // UV + offset(z)
    nointerpolation uint TexIndex   : TEXINDEX;
    nointerpolation float Opacity   : OPACITY;
};

// Texture
Texture2DArray<float4> Textures : TEXTURE : register(t0);
SamplerState sampState   : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    float4 finalColor = Textures.SampleLevel(sampState, float3(input.TexCoord.xy, input.TexIndex), 0);
    finalColor.a *= TEX_STRENGTH * (1.1f - saturate(input.TexCoord.z));
    
    return finalColor * input.Opacity;
}