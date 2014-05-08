#include <common.h.hlsl>

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
    output.uv0 = input.uv0;
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	output.worldPos.xyz = mul(float4(input.position, 1), model).xyz;
	output.worldPos.w = output.position.z;
	return output;
}