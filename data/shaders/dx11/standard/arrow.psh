cbuffer arrowData
{
	matrix modelViewProjection : packoffset(c0);
	float3 position : packoffset(c4);
	float4 orientation : packoffset(c5);
	float4 color : packoffset(c6);
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return color;
}