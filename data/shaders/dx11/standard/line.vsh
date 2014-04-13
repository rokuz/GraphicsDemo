cbuffer lineData
{
    matrix modelViewProjection : packoffset(c0);
    float4 color : packoffset(c4);
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(modelViewProjection, float4(input.position, 1));
    return output;    
}