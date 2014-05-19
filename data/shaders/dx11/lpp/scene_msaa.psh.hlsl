#include <common.h.hlsl>

texture2D diffuseMap : register(t0);
texture2D specularMap : register(t1);
Texture2DMS<float4> lbufferMap : register(t2);
Texture2DMS<float4> sbufferMap : register(t3);
SamplerState defaultSampler;

[earlydepthstencil]
float4 main(VS_OUTPUT_SCENE input, uint sampleIndex : SV_SAMPLEINDEX) : SV_Target0
{
	float4 diffTex = diffuseMap.Sample(defaultSampler, input.uv0);
	float3 specTex = specularMap.Sample(defaultSampler, input.uv0);

	float3 lbuffer = lbufferMap.Load(int2(input.position.xy), sampleIndex);
	float3 sbuffer = sbufferMap.Load(int2(input.position.xy), sampleIndex);

	float3 color = lbuffer * diffTex + sbuffer * specTex;

	return float4(color, diffTex.a);
}