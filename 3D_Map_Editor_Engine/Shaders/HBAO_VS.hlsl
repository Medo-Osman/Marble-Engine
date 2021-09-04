struct VS_IN
{
    float3 Position : POSITION;
    float3 frustumIndex : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
    float4 FrustumVector : POSITION;
    float2 TexCoord : TEXCOORD1;
};

cbuffer FrustumVectors : register(b0)
{
    float4 FrustumVectors[4];
    matrix invProjectionMatrix;
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

VS_OUT main(VS_IN input, uint vertexID : SV_VertexID)
{
    VS_OUT output;

    //output.TexCoord = gQuadUVs[vertexID];

    //// Quad covering screen in NDC space ([-1.0, 1.0] x [-1.0, 1.0] x [0.0, 1.0] x [1.0])
    //output.Position = float4(2.0f * output.TexCoord.x - 1.0f,
    //                            1.0f - 2.0f * output.TexCoord.y,
    //                            0.0f,
    //                            1.0f);

    //// Transform current quad corner to view space.
    //float4 ph = mul(output.Position, invProjectionMatrix);
    //output.FrustumVector = ph / ph.w;
    
    output.Position = float4(input.Position.xy, 0.f, 1.f);
    output.TexCoord = input.TexCoord;
    output.FrustumVector = FrustumVectors[input.frustumIndex.x];

    return output;
}