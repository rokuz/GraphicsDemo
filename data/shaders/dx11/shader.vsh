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
};

cbuffer spaceData
{
	matrix modelViewProjection : packoffset(c0);
	matrix model : packoffset(c4);
	float3 viewDirection : packoffset(c8);
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
    output.uv0 = input.uv0;
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	
	return output;
}