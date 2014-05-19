#include <common.h.hlsl>

texture2D normalMap : register(t0);
SamplerState defaultSampler;

float2 packNormal(float3 normal)
{
	return float2(atan2(normal.y, normal.x), normal.z);
}

float4 main(VS_OUTPUT_GBUF input) : SV_Target0
{
	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0.xy).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
    return float4(length(input.worldViewPos), packNormal(normal), float(materialId));
}