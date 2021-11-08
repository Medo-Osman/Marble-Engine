struct Particle
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float2 size     : SIZE;
    float  rotation : ROTATION;
    float  age      : AGE;
    uint   type     : TYPE;
    uint   maxID    : MAXID;
};

Particle main(Particle input)
{
    return input;
}