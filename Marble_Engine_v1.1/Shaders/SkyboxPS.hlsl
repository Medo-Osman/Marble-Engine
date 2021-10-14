struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION;
};

TextureCube skyboxTexture : register(t6);
SamplerState sampState : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default

float4 main(PixelShaderInput input) : SV_TARGET
{
    return skyboxTexture.Sample(sampState, input.localPos);
}