#include <common.h.hlsl>

struct PS_INPUT
{
    float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 worldPos : TEXCOORD3;
};

texture2D diffuseMap : register(t0);
texture2D normalMap : register(t1);
texture2D specularMap : register(t2);
SamplerState defaultSampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	const float specPower = 10.0;

	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));

	float ndol = max(0, dot(light.direction, normal));

	float3 textureColor = diffuseMap.Sample(defaultSampler, input.uv0).rgb;
	float3 diffuse = textureColor * light.diffuseColor * ndol;

	float3 viewDirection = normalize(input.worldPos - viewPosition);
	float3 h = normalize(viewDirection + light.direction);
	float3 specColor = specularMap.Sample(defaultSampler, input.uv0).rgb;
	float3 specular = specColor * light.specularColor * pow(max(dot(normal, h), 0.0), specPower);
	
	float3 ambient = textureColor * light.ambientColor;
	
    return float4(saturate(ambient + diffuse + specular), 1);
}