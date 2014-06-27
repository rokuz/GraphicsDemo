#include <common.h.hlsl>

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return lerp(float4(1,0,0,1), float4(0,0,1,1), input.uv0.x);
}