#include <common.h.hlsl>
#include <pscommon.h.hlsl>

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float3 color = computeColorOpaque(input);
    return float4(color, 1);
}