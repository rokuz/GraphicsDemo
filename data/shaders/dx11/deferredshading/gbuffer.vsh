#include <common.h>

VS_OUTPUT_GBUF main(VS_INPUT input)
{
	VS_OUTPUT_GBUF output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
    output.uv0 = input.uv0;
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	output.worldPos = mul(float4(input.position, 1), model).xyz;
	
	return output;
}