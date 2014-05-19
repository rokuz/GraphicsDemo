#include <common.h.hlsl>

VS_OUTPUT_SCENE main(VS_INPUT input)
{
	VS_OUTPUT_SCENE output;
    output.position = mul(float4(input.position, 1), modelViewProjection);
	output.uv0 = input.uv0;
	
	return output;
}