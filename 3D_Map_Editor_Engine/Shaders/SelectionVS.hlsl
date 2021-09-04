struct VS_IN
{
    float3 position     : POSITION;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BITANGENT;
    float2 texCoord     : TEXCOORD;
};

struct VS_OUT
{
    float4 position     : SV_POSITION;
};

cbuffer constantBuffer : register(b0)
{
    matrix wvpMatrix;
    matrix worldMatrix;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    output.position = mul(float4(input.position, 1.f), wvpMatrix);
    
    return output;
}