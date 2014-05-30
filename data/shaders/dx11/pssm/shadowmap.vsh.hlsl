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
	float depth : TEXCOORD0;
	uint instanceID : SV_InstanceID;
};

cbuffer entityData : register(b0)
{
	matrix modelViewProjection;
	matrix model;
};

static const int MAX_SPLITS = 8;
cbuffer shadowData : register(b1)
{
	matrix shadowViewProjection[MAX_SPLITS];
};

VS_OUTPUT main(VS_INPUT input, unsigned int instanceID : SV_InstanceID)
{
	VS_OUTPUT output;
	float4 pos = mul(float4(input.position, 1), model);
    output.position = mul(pos, shadowViewProjection[instanceID]);
    output.depth = output.position.z;
	output.instanceID = instanceID;
	
	return output;
}