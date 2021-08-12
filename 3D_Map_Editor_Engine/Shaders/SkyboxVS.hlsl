struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION;
};

cbuffer constantBuffer : register(b0)
{
    matrix vpMatrix; // Includes only rotation if world is multiplied
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(vpMatrix, float4(input.pos, 0.f)).xyww;
    output.localPos = input.pos;

    return output;
}