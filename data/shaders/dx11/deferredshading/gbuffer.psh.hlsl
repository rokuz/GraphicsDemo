#include <common.h.hlsl>

texture2D diffuseMap : register(t0);
texture2D normalMap : register(t1);
texture2D specularMap : register(t2);
SamplerState defaultSampler;

struct PS_OUTPUT_GBUF
{
	float4 dataBlock1 : SV_Target0;
	uint3 dataBlock2 : SV_Target1;
};

uint packColor(float4 color)
{
	return (uint(color.r * 255) << 24) | (uint(color.g * 255) << 16) | (uint(color.b * 255) << 8) | uint(color.a * 255);
}

float2 packNormal(float3 normal)
{
	return float2(atan2(normal.y, normal.x), normal.z);
}

PS_OUTPUT_GBUF main(VS_OUTPUT_GBUF input, uint coverage : SV_COVERAGE)
{
	PS_OUTPUT_GBUF output = (PS_OUTPUT_GBUF)0;
	
	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0.xy).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	
	output.dataBlock1 = float4(length(input.worldViewPos), packNormal(normal), specularPower);
	output.dataBlock2.x = (materialId & 0x0000ffff) | (coverage << 16);
	output.dataBlock2.y = packColor(diffuseMap.Sample(defaultSampler, input.uv0.xy));
	output.dataBlock2.z = packColor(specularMap.Sample(defaultSampler, input.uv0.xy));

    return output;
}