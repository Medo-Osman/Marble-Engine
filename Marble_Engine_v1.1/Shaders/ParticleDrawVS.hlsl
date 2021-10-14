struct Particle
{
    float3  initialPosition : POSITION;
    float3  initialVelocity : VELOCITY;
    float2  size            : SIZE;
    float   age             : AGE;
    uint    type            : TYPE;
};

struct VS_OUT
{
    float3  position    : POSITION;
    float2  size        : SIZE;
    float4  color       : COLOR;
    uint    type        : TYPE;
};

VS_OUT main(Particle input)
{
    VS_OUT output;
    
    float age = input.age;
    float3 acceleration = { 0.0f, 7.8f, 0.0f };
    
    // constant acceleration equation
    output.position = 0.5f * age * age * acceleration + age * input.initialVelocity + input.initialPosition;
    
    // fade color with time
    float opacity = 1.0f - smoothstep(0.0f, 1.0f, age / 1.0f);
    output.color = float4(1.0f, 1.0f, 1.0f, opacity);
    
    output.size = input.size;
    output.type = input.type;
    
    return output;
}