#include <common.h.hlsl>

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 worldPos : TEXCOORD3;
};

texture2D diffuseMap : register(t1);
texture3D furMap : register(t2);
SamplerState defaultSampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	const float specPower = 30.0;

	float3 coords = input.uv0 * float3(50.0, 50.0, 1.0f);
	float4 fur = furMap.Sample(defaultSampler, coords);
	clip(fur.a - 0.01);

	float4 outputColor = float4(0, 0, 0, 0);
	outputColor.a = fur.a * (1.0 - input.uv0.z);

	outputColor.rgb = diffuseMap.Sample(defaultSampler, input.uv0.xy).rgb;

	float3 viewDirection = normalize(input.worldPos - viewPosition);
	
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 tangentVector = normalize((fur.rgb - 0.5f) * 2.0f);
	tangentVector = normalize(mul(tangentVector, ts));

	float TdotL = dot(tangentVector, light.direction);
	float TdotE = dot(tangentVector, viewDirection);
	float sinTL = sqrt(1 - TdotL * TdotL);
	float sinTE = sqrt(1 - TdotE * TdotE);
	outputColor.xyz = light.ambientColor * outputColor.rgb +
					  light.diffuseColor * (1.0 - sinTL) * outputColor.rgb +
					  light.specularColor * pow(abs((TdotL * TdotE + sinTL * sinTE)), specPower) * 0.2;

	float minShadow = 1.0;
	float shadow = input.uv0.z * (1.0f - minShadow) + minShadow;
	outputColor.rgb *= shadow;

	return outputColor;
}