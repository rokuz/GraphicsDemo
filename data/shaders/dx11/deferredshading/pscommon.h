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

void processLight(const uint lightIndex, float3 worldPos, out float3 lightDir, out float lightPower)
{
	[branch]
	if (lightsData[lightIndex].lightType == 0) // omni light
	{
		float3 ldv = worldPos - lightsData[lightIndex].position;
		lightDir = normalize(ldv);
		float falloff = 1.0 - saturate(length(ldv) / (lightsData[lightIndex].falloff + 1e-7));
		lightPower = falloff * falloff;
	}
	else if (lightsData[lightIndex].lightType == 1) // spot light
	{
		float3 ldv = worldPos - lightsData[lightIndex].position;
		lightDir = normalize(ldv);
		
		float cosAng = dot(lightDir, lightsData[lightIndex].direction);
		float cosOuter = cos(lightsData[lightIndex].angle);
		lightPower = smoothstep(cosOuter, cosOuter + 0.1, cosAng);
		
		[branch]
		if (lightPower > 1e-5)
		{
			float falloff = 1.0 - saturate(length(ldv) / (lightsData[lightIndex].falloff + 1e-7));
			lightPower *= (falloff * falloff);
		}
	}
	else // direct light
	{
		lightDir = lightsData[lightIndex].direction;
		lightPower = 1;
	}
}

void blinn(float3 normal, float3 worldPos, out float3 diffColor, out float3 specColor, out float3 ambColor)
{
	const float specPower = 30.0;
	
	float3 viewDir = normalize(worldPos - viewPosition);

	diffColor = float3(0, 0, 0);
	specColor = float3(0, 0, 0);
	ambColor = float3(0, 0, 0);
	for (uint i = 0; i < lightsCount; i++)
	{
		float3 lightDir;
		float lightPower;
		processLight(i, worldPos, lightDir, lightPower);
		
		float ndol = max(0.0, dot(lightDir, normal));
		diffColor += lightsData[i].diffuseColor * ndol * lightPower;
		
		float3 h = normalize(viewDir + lightDir);
		specColor += lightsData[i].specularColor * pow (max(dot(normal, h), 0.0), specPower) * lightPower;
		
		ambColor += lightsData[i].ambientColor * lightPower;
	}
}

float3 computeColor(VS_OUTPUT_GBUF input)
{
	float3 normalTS = normalMap.Sample(defaultSampler, input.uv0.xy).rgb * 2.0 - 1.0;
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));

	float3 diffColor, specColor, ambColor;
	blinn(normal, input.worldPos, diffColor, specColor, ambColor);
	
	float3 diffTex = diffuseMap.Sample(defaultSampler, input.uv0.xy).rgb;
	float3 diffuse = diffTex * diffColor;
	float3 specular = specularMap.Sample(defaultSampler, input.uv0.xy).rgb * specColor;
	float3 ambient = diffTex * ambColor;
	
	return saturate(ambient + diffuse + specular);
}