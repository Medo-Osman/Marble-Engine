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
    float4 position         : SV_POSITION;
    float3 wPosition        : POSITION;
    float4 shadowPosition   : POSITION1;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BITANGENT;
    float2 texCoord         : TEXCOORD;
};

cbuffer constantBuffer : register(b0)
{
    matrix wvpMatrix;
    matrix worldMatrix;
};

cbuffer lightSpaceMatrices : register(b1)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    output.position = mul(wvpMatrix, float4(input.position, 1.f));
    output.wPosition = mul(worldMatrix, float4(input.position, 1.f));
    
    output.shadowPosition = mul(worldMatrix, float4(input.position, 1.f));
    output.shadowPosition = mul(lightViewMatrix, output.shadowPosition);
    output.shadowPosition = mul(lightProjectionMatrix, output.shadowPosition);
    
    output.normal = normalize(mul(worldMatrix, float4(input.normal, 0.f)).xyz);
    output.tangent = normalize(mul(worldMatrix, float4(input.tangent, 0.f)).xyz);
    output.biTangent = normalize(mul(worldMatrix, float4(input.biTangent, 0.f)).xyz);
    output.texCoord = input.texCoord;
    
    return output;
}