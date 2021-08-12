struct Particle
{
    float3  initialPosition : POSITION;
    float3  initialVelocity : VELOCITY;
    float2  size            : SIZE;
    float   age             : AGE;
    uint    type            : TYPE;
};

Particle main(Particle input)
{
    return input;
}