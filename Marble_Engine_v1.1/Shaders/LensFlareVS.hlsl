struct VS_OUT
{
    float4 Position                 : SV_POSITION;
    nointerpolation uint VertexID   : VERTEXID;
};

VS_OUT main(float4 position : POSITION, uint vertexID : SV_VERTEXID)
{
    VS_OUT output;
    output.Position = position;
    output.VertexID = vertexID;
    
    return output;
}