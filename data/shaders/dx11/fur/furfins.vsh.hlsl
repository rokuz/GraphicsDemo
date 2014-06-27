#include <common.h.hlsl>

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.position = float4(input.position, 1);
	output.normal = normalize(input.normal);
	return output;
}