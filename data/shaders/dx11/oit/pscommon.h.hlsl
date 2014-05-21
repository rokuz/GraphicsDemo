struct LightData
{
	float3 position;
	uint lightType;
	float3 direction;
	float falloff;
	float3 diffuseColor;
	float angle;
	float3 ambientColor;
	uint dummy;
	float3 specularColor;
	uint dummy2;
};
StructuredBuffer<LightData> lightsData : register(t0);

TextureCube environmentMap : register(t1);
SamplerState defaultSampler;

void blinn(float3 normal, float3 viewDir, out float3 diffColor, out float3 specColor, out float3 ambColor)
{
	const float specPower = 30.0;

	diffColor = float3(0, 0, 0);
	specColor = float3(0, 0, 0);
	ambColor = float3(0, 0, 0);
	for (uint i = 0; i < lightsCount; i++)
	{
		float ndol = max(0.0, dot(lightsData[i].direction, normal));
		diffColor += lightsData[i].diffuseColor * ndol;
		
		float3 h = normalize(viewDir + lightsData[i].direction);
		specColor += lightsData[i].specularColor * pow (max(dot(normal, h), 0.0), specPower);
		
		ambColor += lightsData[i].ambientColor;
	}
}

float3 computeColorOpaque(VS_OUTPUT input)
{
	float3 normalTS = float3(0, 0, 1);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	
	float3 viewDir = normalize(input.worldPos.xyz - viewPosition);
	float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);
	
	float3 reflectVec = reflect(viewDir, normal);
	float3 envColor = environmentMap.Sample(defaultSampler, reflectVec).rgb;
	float3 diffuse = envColor * (diffColor + ambColor);
	
	return saturate(diffuse + specColor);
}

float4 computeColorTransparent(VS_OUTPUT input, bool frontFace)
{
	float3 normalTS = float3(0, 0, 1);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = normalize(mul(normalTS, ts)) * (frontFace ? -1.0f : 1.0f);

	float3 viewDir = normalize(input.worldPos.xyz - viewPosition);
	float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);

	float3 reflectVec = reflect(viewDir, normal);
	float3 envColor = environmentMap.Sample(defaultSampler, reflectVec).rgb;
	float alpha = clamp(1.0f - dot(viewDir, normal), 0.3f, 1.0f);

	float3 diffuse = envColor * (diffColor + ambColor);

	return float4(saturate(diffuse + specColor), alpha);
}