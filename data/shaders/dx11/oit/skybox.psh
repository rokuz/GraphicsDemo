#include <common.h>

struct VS_OUTPUT_SKYBOX
{
    float4 position : SV_POSITION;
	float3 uv : TEXCOORD0;
};

TextureCube skyboxMap;
SamplerState defaultSampler;

float4 main(VS_OUTPUT_SKYBOX input) : SV_TARGET
{
	float3 color = saturate(skyboxMap.Sample(defaultSampler, input.uv));
	return float4(color, 1);
}