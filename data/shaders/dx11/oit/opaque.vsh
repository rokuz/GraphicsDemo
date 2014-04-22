#include <common.h>

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
    output.uv0_depth = float3(input.uv0, output.position.z);
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	float3 worldPos = mul(float4(input.position, 1), model);
	
	return output;
}