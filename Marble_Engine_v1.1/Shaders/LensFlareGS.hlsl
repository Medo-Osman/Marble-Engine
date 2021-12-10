// Globals
static const float OFFSETS[] = { 1.f, 0.75f, 0.5f, 0.25f, 0.f, -0.25f, -0.5f, -0.75f, -1.0 };
static const float2 TEX_SIZE = float2(256.f, 256.f);

// Vertex
struct GS_IN
{
    float4 Position                 : SV_POSITION;
    nointerpolation uint VertexID   : VERTEXID;
};

struct GS_OUT
{
    float4 Position                 : SV_POSITION;
    float3 TexCoord                 : TEXCOORD0; // UV + offset(z)
    nointerpolation uint TexIndex   : TEXINDEX;
    nointerpolation float Opacity   : OPACITY;
};

// ConstantBuffer
cbuffer LensFlareCB : register(b2)
{
    float4 sunPosition; // In Screen Space
    float2 screenDimensions; // Used for Depth Texture dimensions
};

// Textures
Texture2D DepthTexture : register(t1);
SamplerState defaultSampler : register(s1);
SamplerComparisonState compareSampler : register(s2); // Same as Shadow Sampler

// Functions
void append(inout TriangleStream<GS_OUT> triStream, GS_OUT output, uint texIndex, float2 posMod, float2 size)
{
    float2 pos = sunPosition.xy;
    float2 moddedPos = pos * posMod;
    float dist = distance(pos, moddedPos);
 
    output.Position.xy      = moddedPos + float2(-size.x,-size.y);
    output.TexCoord.z       = dist;
    output.TexIndex         = texIndex;
    output.TexCoord.xy      = float2(0.f, 0.f);
    triStream.Append(output);
 
    output.Position.xy      = moddedPos + float2(-size.x, size.y);
    output.TexCoord.xy      = float2(0.f, 1.f);
    triStream.Append(output);
 
    output.Position.xy      = moddedPos + float2(size.x,-size.y);
    output.TexCoord.xy      = float2(1.f, 0.f);
    triStream.Append(output);
 
    output.Position.xy      = moddedPos + float2(size.x, size.y);
    output.TexCoord.xy      = float2(1.f, 1.f);
    triStream.Append(output);
}

// Main
[maxvertexcount(4)]
void main(point GS_IN input[1], inout TriangleStream<GS_OUT> triStream)
{
    if (sunPosition.z < 0.f)
        return;
    
    GS_OUT output = (GS_OUT) 0;
    
    float2 flareSize = TEX_SIZE / screenDimensions;
    const float2 step = 1.0f / (screenDimensions * sunPosition.z);
    const float2 range = 10.f * step;
    float samples = 0.0f;
    float accumulation = 0.0f;
 
    float2 sunPosUV = sunPosition.xy / sunPosition.w * 0.5f + 0.5f;
    sunPosUV.y = 1.0f - sunPosUV.y;
    
    for (float y = -range.y; y <= range.y; y += step.y)
    {
        for (float x = -range.x; x <= range.x; x += step.x)
        {
            samples += 1.0f;
            accumulation += (DepthTexture.SampleCmpLevelZero(compareSampler, sunPosUV + float2(x, y), sunPosition.z).r);
        }
    }
    accumulation /= samples;
    
    output.Position = float4(0, 0, 0, 1);
    output.Opacity = accumulation;
    
    if (accumulation)
        append(triStream, output, input[0].VertexID, OFFSETS[input[0].VertexID], flareSize);
}