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

texture2D diffuseMap : register(t1);
texture2D normalMap : register(t2);
texture2D specularMap : register(t3);
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

float3 computeColor(VS_OUTPUT input, const bool useDiffTex)
{
	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0.xy).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	
	float3 viewDir = normalize(input.worldPos.xyz - viewPosition);
	float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);
	
	float3 diffTex = useDiffTex ? diffuseMap.Sample(defaultSampler, input.uv0.xy).rgb : objectColor;
	float3 diffuse = diffTex * diffColor;
	float3 specular = specularMap.Sample(defaultSampler, input.uv0.xy).rgb * specColor;
	float3 ambient = diffTex * ambColor;
	
	return saturate(ambient + diffuse + specular);
}