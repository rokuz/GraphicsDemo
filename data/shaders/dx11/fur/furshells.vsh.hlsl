#include <common.h.hlsl>

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 worldPos : TEXCOORD3;
};

texture2D furLengthMap : register(t0);
texture3D furMap : register(t2);
SamplerState defaultSampler : register(s0);

static const float FUR_LENGTH = 0.03f;

VS_OUTPUT main(VS_INPUT input, unsigned int instID : SV_InstanceID)
{
	VS_OUTPUT output;
	float furLen = furLengthMap.SampleLevel(defaultSampler, input.uv0, 0).r;
	float3 furDir = normalize((furMap.SampleLevel(defaultSampler, float3(input.uv0 * 50.0, 0), 0) - 0.5f) * 2.0f);

	float3 pos = input.position.xyz + normalize(input.normal) * furLen * FUR_LENGTH * float(instID + 1) / FUR_LAYERS;
	output.position = mul(float4(pos, 1), modelViewProjection);
	output.uv0 = float3(input.uv0, float(instID + 1) / FUR_LAYERS);
	output.normal = mul(normalize(input.normal), (float3x3)model);
	output.tangent = mul(normalize(input.tangent), (float3x3)model);
	output.worldPos = mul(float4(pos, 1), model);

	return output;
}