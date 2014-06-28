#include <common.h.hlsl>

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = float4(input.position, 1);
	output.uv0 = input.uv0;
	output.normal = normalize(input.normal);
	return output;
}