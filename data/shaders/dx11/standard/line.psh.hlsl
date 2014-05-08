cbuffer lineData
{
    matrix modelViewProjection : packoffset(c0);
    float4 color : packoffset(c4);
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return color;
}