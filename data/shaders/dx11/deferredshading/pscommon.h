struct LightData
{
	float3 positionOrDirection;
	uint lightType;
	float3 diffuseColor;
	float falloff;
	float3 ambientColor;
	float angle;
	float3 specularColor;
};
StructuredBuffer<LightData> lightsData;

texture2D diffuseMap;
texture2D normalMap;
texture2D specularMap;
SamplerState defaultSampler;

void blinn(float3 normal, float3 viewDir, out float3 diffColor, out float3 specColor, out float3 ambColor)
{
	const float specPower = 30.0;

	diffColor = float3(0, 0, 0);
	specColor = float3(0, 0, 0);
	ambColor = float3(0, 0, 0);
	for (uint i = 0; i < lightsCount; i++)
	{
		float ndol = max(0.0, dot(lightsData[i].positionOrDirection, normal));
		diffColor += lightsData[i].diffuseColor * ndol;
		
		float3 h = normalize(viewDir + lightsData[i].positionOrDirection);
		specColor += lightsData[i].specularColor * pow (max(dot(normal, h), 0.0), specPower);
		
		ambColor += lightsData[i].ambientColor;
	}
}

float3 computeColor(VS_OUTPUT_GBUF input)
{
	float3 normalTS = normalMap.Sample(defaultSampler, input.uv0.xy).rgb * 2.0 - 1.0;
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	
	float3 viewDir = normalize(viewPosition - input.worldPos);
	float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);
	
	float3 diffTex = diffuseMap.Sample(defaultSampler, input.uv0.xy).rgb;
	float3 diffuse = diffTex * diffColor;
	float3 specular = specularMap.Sample(defaultSampler, input.uv0.xy).rgb * specColor;
	float3 ambient = diffTex * ambColor;
	
	return saturate(ambient + diffuse + specular);
}