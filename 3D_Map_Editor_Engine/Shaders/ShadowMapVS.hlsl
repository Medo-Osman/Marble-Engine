struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
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

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 outTextureCord : TEXCOORD;
};
 
VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    // Transform the vertex position into projected space.
    //matrix lightWVPMatrix = worldMatrix * lightViewMatrix * lightProjectionMatrix;
    //output.position = mul(lightWVPMatrix, float4(input.position, 1.0f));
    //output.position = mul(wvpMatrix, float4(input.position, 1.f));
    
    output.position = mul(float4(input.position, 1.f), worldMatrix);
    output.position = mul(output.position, lightViewMatrix);
    output.position = mul(output.position, lightProjectionMatrix);
    
    //output.position = mul(float4(input.position, 1.0f), worldMatrix * lightViewMatrix * lightProjectionMatrix);
    
    //output.outTextureCord = mul(textureTransformMatrix, float4(input.texCoord, 0.0f, 1.0f)).xy;
    output.outTextureCord = input.texCoord;

    return output;
}