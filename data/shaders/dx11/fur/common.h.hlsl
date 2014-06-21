cbuffer entityData : register(b0)
{
	matrix modelViewProjection;
	matrix model;
};

cbuffer onFrameData : register(b1)
{
	float3 viewPosition;
	uint onFrameData_dummy1;
};

struct LightData
{
	float3 position;
	uint lightType;
	float3 direction;
	float falloff;
	float3 diffuseColor;
	float angle;
	float3 ambientColor;
	uint dummy;
	float3 specularColor;
	uint dummy2;
};
cbuffer lightsData : register(b2)
{
	LightData light;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};
