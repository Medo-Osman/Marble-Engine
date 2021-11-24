struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION;
};

cbuffer WVPBuffer : register(b0)
{
    matrix vpMatrix; // Includes only rotation if world is multiplied
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    output.pos = mul(float4(input.pos, 0.f), vpMatrix);
    output.localPos = input.pos;
    output.pos = output.pos.xyww;

    return output;
}