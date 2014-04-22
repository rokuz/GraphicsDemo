struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float3 uv0_depth : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
};

cbuffer spatialData
{
	matrix modelViewProjection : packoffset(c0);
	matrix model : packoffset(c4);
	float3 viewDirection : packoffset(c8);
	uint lightsCount : packoffset(c8.w);
};