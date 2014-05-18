#version 430 core

in VS_OUTPUT_GBUF
{
	vec2 uv0;
	vec3 normal;
	vec3 tangent;
	vec3 worldPos;
} psinput;

out vec4 outputColor;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform float specularPower;

// lights
const int MAX_LIGHTS_COUNT = 8;
struct LightData
{
	vec3 position;
	uint lightType;
	vec3 direction;
	float falloff;
	vec3 diffuseColor;
	float angle;
	vec3 ambientColor;
	uint dummy;
	vec3 specularColor;
	uint dummy2;
};
layout(std140) uniform lightsDataBuffer
{
    LightData lightsData[MAX_LIGHTS_COUNT];
};

uniform uint lightsCount;

uniform vec3 viewPosition;

void processLight(const uint lightIndex, vec3 worldPos, out vec3 lightDir, out float lightPower)
{
	if (lightsData[lightIndex].lightType == 0) // omni light
	{
		vec3 ldv = worldPos - lightsData[lightIndex].position;
		lightDir = normalize(ldv);
		float falloff = 1.0 - clamp(length(ldv) / (lightsData[lightIndex].falloff + 1e-7), 0.0f, 1.0f);
		lightPower = falloff * falloff;
	}
	else if (lightsData[lightIndex].lightType == 1) // spot light
	{
		vec3 ldv = worldPos - lightsData[lightIndex].position;
		lightDir = normalize(ldv);
		
		float cosAng = dot(lightDir, lightsData[lightIndex].direction);
		float cosOuter = cos(lightsData[lightIndex].angle);
		lightPower = smoothstep(cosOuter, cosOuter + 0.1, cosAng);
		
		if (lightPower > 1e-5)
		{
			float falloff = 1.0 - clamp(length(ldv) / (lightsData[lightIndex].falloff + 1e-7), 0.0f, 1.0f);
			lightPower *= (falloff * falloff);
		}
	}
	else // direct light
	{
		lightDir = lightsData[lightIndex].direction;
		lightPower = 1;
	}
}

void blinn(in vec3 normal, in vec3 worldPos, in float specPower, out vec3 diffColor, out vec3 specColor, out vec3 ambColor)
{
	vec3 viewDir = normalize(worldPos - viewPosition);

	diffColor = vec3(0, 0, 0);
	specColor = vec3(0, 0, 0);
	ambColor = vec3(0, 0, 0);
	
	for (uint i = 0; i < lightsCount; i++)
	{
		vec3 lightDir;
		float lightPower;
		processLight(i, worldPos, lightDir, lightPower);
		
		// skip weak lights
		if (lightPower > 0.01)
		{
			float ndol = max(0.0, dot(lightDir, normal));
			diffColor += lightsData[i].diffuseColor * ndol * lightPower;
			
			vec3 h = normalize(viewDir + lightDir);
			specColor += lightsData[i].specularColor * pow(max(dot(normal, h), 0.0), specPower) * lightPower;
			
			ambColor += lightsData[i].ambientColor * lightPower;
		}
	}
}

void main()
{
	vec3 normalTS = normalize(texture(normalMap, psinput.uv0).rgb * 2.0 - 1.0);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	vec3 normal = -normalize(ts * normalTS);
	
	vec4 diffTex = texture(diffuseMap, psinput.uv0);
	vec4 specularTex = texture(specularMap, psinput.uv0);

	// lighting
	vec3 diffColor, specColor, ambColor;
	blinn(normal, psinput.worldPos, specularPower, diffColor, specColor, ambColor);
	
	vec3 diffuse = diffTex.rgb * diffColor;
	vec3 specular = specularTex.rgb * specColor;
	vec3 ambient = diffTex.rgb * ambColor;
	
	outputColor = vec4(clamp(ambient + diffuse + specular, 0.0f, 1.0f), diffTex.a);
}