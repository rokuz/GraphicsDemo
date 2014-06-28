#include <common.h.hlsl>

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 uv0 : TEXCOORD0;
};

texture2D diffuseMap : register(t1);
texture3D furMap : register(t2);
SamplerState defaultSampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 color = diffuseMap.Sample(defaultSampler, input.uv0.xy).rgb;
	float3 coords = input.uv0 * float3(FUR_SCALE, FUR_SCALE, 1.0f);
	float4 fur = furMap.Sample(defaultSampler, coords);
	clip(fur.a - 0.01);
	float alpha = fur.a * (1.0 - input.uv0.z);

	float shadow = input.uv0.z * (1.0f - FUR_SELF_SHADOWING) + FUR_SELF_SHADOWING;
	color.rgb *= shadow;

	return float4(color, alpha);
}