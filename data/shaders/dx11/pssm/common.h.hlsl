static const int MAX_SPLITS = 4;

cbuffer entityData : register(b0)
{
	matrix modelViewProjection;
	matrix model;
	int4 shadowIndices;
};

cbuffer onFrameData : register(b1)
{
	float3 viewPosition;
	int splitsCount;
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

cbuffer shadowData : register(b3)
{
	matrix shadowViewProjection[MAX_SPLITS];
};

static const float SHADOW_BIASES[MAX_SPLITS] = 
{ 
	0.0009, 0.002, 0.0025, 0.003
};

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};
