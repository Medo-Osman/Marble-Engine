#define OFFSET_X 70
#define OFFSET_Y 70

struct PixelShaderInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PixelShaderOuput
{
    float4 skyboxRT : SV_Target0;
    float4 irradianceRT : SV_Target1;
};

TextureCube skyboxTexture : register(t0);
TextureCube irradianceTexture : register(t1);
SamplerState sampState : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default

PixelShaderOuput main(PixelShaderInput input)
{
    PixelShaderOuput output;
    
    float uIndex = (input.Position.x / OFFSET_X);
    float vIndex = -(input.Position.y / OFFSET_Y) + 0.5;
    uint wIndex = 1;
    float3 texCoord = float3(uIndex, vIndex, wIndex);
    
    output.skyboxRT = skyboxTexture.Sample(sampState, texCoord);
    output.irradianceRT = irradianceTexture.Sample(sampState, texCoord);
    
    return output;
}