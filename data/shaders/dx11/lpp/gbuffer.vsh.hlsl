#include <common.h.hlsl>

VS_OUTPUT_GBUF main(VS_INPUT input)
{
	VS_OUTPUT_GBUF output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
	output.uv0 = input.uv0;
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	output.worldViewPos = mul(float4(input.position, 1), modelView).xyz;
	
	return output;
}