struct VS_IN
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_OUT
{
    float3 wPosition : POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer lightSpaceMatrices : register(b1)
{
    matrix invLightViewProjectionMatrix;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    float4 worldPos = mul(float4(input.position, 1.f), invLightViewProjectionMatrix);
    output.wPosition = (worldPos * (1.f / worldPos.w)).xyz;
    output.texCoord = input.texCoord;
    
    return output;
}