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

cbuffer WVPBuffer : register(b0)
{
    matrix wvpMatrix;
    matrix worldMatrix;
    matrix normalMatrix;
};

cbuffer lightSpaceMatrices : register(b1)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    output.position = mul(float4(input.position, 1.f), wvpMatrix);
    output.wPosition = mul(float4(input.position, 1.f), worldMatrix);
    
    output.shadowPosition = mul(float4(input.position, 1.f), worldMatrix);
    output.shadowPosition = mul(output.shadowPosition, lightViewMatrix);
    output.shadowPosition = mul(output.shadowPosition, lightProjectionMatrix);
    
    output.normal = normalize(mul(input.normal, (float3x3) normalMatrix).xyz);
    output.tangent = normalize(mul(input.tangent, (float3x3) normalMatrix).xyz);
    output.biTangent = normalize(mul(input.biTangent, (float3x3) normalMatrix).xyz);
    output.texCoord = input.texCoord;
    
    return output;
}