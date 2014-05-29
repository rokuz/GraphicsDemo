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
	float2 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 viewDirection : TEXCOORD3;
};

cbuffer onFrameData : register(b0)
{
	float3 viewPosition;
	uint onFrameData_dummy;
};

cbuffer entityData : register(b1)
{
	matrix modelViewProjection;
	matrix model;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
    output.uv0 = input.uv0;
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	float3 worldPos = mul(float4(input.position, 1), model);
	output.viewDirection = normalize(worldPos - viewPosition);
	
	return output;
}