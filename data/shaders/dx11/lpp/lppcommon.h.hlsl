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

void blinn(in float3 normal, in float3 worldPos, in float specPower, out float3 diffColor, out float3 specColor, out float3 ambColor)
{
	float3 viewDir = normalize(worldPos - viewPosition);

	diffColor = float3(0, 0, 0);
	specColor = float3(0, 0, 0);
	ambColor = float3(0, 0, 0);
	
	for (uint i = 0; i < lightsCount; i++)
	{
		float3 lightDir;
		float lightPower;
		processLight(i, worldPos, lightDir, lightPower);
		
		// skip weak lights
		[branch]
		if (lightPower > 0.01)
		{
			float ndol = max(0.0, dot(lightDir, normal));
			diffColor += lightsData[i].diffuseColor * ndol * lightPower;
			
			float3 h = normalize(viewDir + lightDir);
			specColor += lightsData[i].specularColor * pow(max(dot(normal, h), 0.0), specPower) * lightPower;
			
			ambColor += lightsData[i].ambientColor * lightPower;
		}
	}
}

struct PS_INPUT_DS
{
    float4 position : SV_POSITION;
	float3 viewRay : TEXCOORD0;
};

struct PS_OUTPUT_DS
{
    float3 lightBuffer : SV_Target0;
	float3 specularBuffer : SV_Target1;
};

float3 unpackNormal(float2 normal)
{
	float2 theta;
	sincos(normal.x, theta.x, theta.y);
	float2 phi = float2(sqrt(1.0f - normal.y * normal.y), normal.y);
	return float3(theta.y * phi.x, theta.x * phi.x, phi.y);
}

float4 unpackPosition(float3 viewRay, float l)
{
	return float4(viewRay * l, 1);
}

static const int MAX_MATERIALS_COUNT = 2;

static const float SPECULAR_POWER[MAX_MATERIALS_COUNT] = 
{ 
	0.0f, 30.0f 
};