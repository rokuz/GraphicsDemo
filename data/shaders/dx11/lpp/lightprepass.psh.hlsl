#include <common.h.hlsl>
#include <lppcommon.h.hlsl>

Texture2D<float4> gbufferMap : register(t1);

PS_OUTPUT_DS main(PS_INPUT_DS input)
{
	PS_OUTPUT_DS output = (PS_OUTPUT_DS)0;
	float4 gbuffer = gbufferMap.Load(int3(input.position.xy, 0));
	clip(gbuffer.w - 0.1);
	uint materialId = uint(gbuffer.w);
	
	float3 nvr = normalize(input.viewRay);
	float3 worldPos = mul(unpackPosition(nvr, gbuffer.x), viewInverse).xyz;
	float3 normal = unpackNormal(gbuffer.yz);

	float3 diffColor, specColor, ambColor;
	blinn(normal, worldPos, SPECULAR_POWER[materialId], diffColor, specColor, ambColor);
	
	output.lightBuffer = diffColor + ambColor;
	output.specularBuffer = specColor;

    return output;
}