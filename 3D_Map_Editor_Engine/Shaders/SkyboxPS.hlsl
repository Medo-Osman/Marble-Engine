struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION;
};

TextureCube skyboxTexture : register(t0);
SamplerState sampState : SAMPLER : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    return skyboxTexture.Sample(sampState, input.localPos);
}