struct VS_OUT
{
    float4 Position : SV_POSITION;
    float4 PositionV : POSITION;
    float2 TexCoord : TEXCOORD1;
};

cbuffer MatrixBuffer : register(b0)
{ 
    //matrix viewToTexMatrix;
    matrix projectionMatrix; // the local projection matrix
    matrix invProjectionMatrix; // the inverse of the local projection matrix
    matrix viewMatrix; // the local view matrix
    float4 viewFrustumVectors[4]; // Frustum Corners
    float2 renderTargetResolution;
};

static const float2 gQuadUVs[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

static const uint frustumIndex[6] = { 1, 0, 3, 1, 3, 2 };

VS_OUT main(uint vertexID : SV_VertexID)
{
    VS_OUT output;

    output.TexCoord = gQuadUVs[vertexID];
    output.Position = float4(2.0f * output.TexCoord.x - 1.0f, 1.0f - 2.0f * output.TexCoord.y, 0.0f, 1.0f);
    
    //float4 ph = mul(output.Position, invProjectionMatrix);
    //output.PositionV = ph / ph.w;
    output.PositionV = viewFrustumVectors[frustumIndex[vertexID]];
    
    return output;
}