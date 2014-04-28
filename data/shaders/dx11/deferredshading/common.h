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

cbuffer onFrameData : register(b0)
{
	float3 viewPosition;
	uint lightsCount;
	uint2 screenSize;
	uint samplesCount;
	uint onFrameData_dummy;
};

cbuffer entityData : register(b1)
{
	matrix modelViewProjection;
	matrix model;
	float specularPower;
	uint materialId;
	uint2 entityData_dummy1;
};