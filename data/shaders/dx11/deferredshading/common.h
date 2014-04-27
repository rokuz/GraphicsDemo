struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct VS_OUTPUT_GBUF
{
    float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 worldPos : TEXCOORD3;
};

cbuffer entityData
{
	matrix modelViewProjection : packoffset(c0);
	matrix model : packoffset(c4);
};

cbuffer onFrameData
{
	float3 viewPosition : packoffset(c0);
	uint lightsCount : packoffset(c0.w);
};