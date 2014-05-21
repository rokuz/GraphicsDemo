#version 430 core

in VS_OUTPUT
{
	vec2 uv0;
	vec3 normal;
	vec3 tangent;
	vec4 worldPos;
} psinput;

out vec4 outputColor;

// lights
const int MAX_LIGHTS_COUNT = 16;
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

uniform samplerCube environmentMap;
uniform vec3 viewPosition;
uniform uint lightsCount;

void blinn(vec3 normal, vec3 viewDir, out vec3 diffColor, out vec3 specColor, out vec3 ambColor)
{
	const float specPower = 30.0;

	diffColor = vec3(0);
	specColor = vec3(0);
	ambColor = vec3(0);
	for (uint i = 0; i < lightsCount; i++)
	{
		float ndol = max(0.0, dot(lightsData[i].direction, normal));
		diffColor += lightsData[i].diffuseColor * ndol;
		
		vec3 h = normalize(viewDir + lightsData[i].direction);
		specColor += lightsData[i].specularColor * pow(max(dot(normal, h), 0.0), specPower);
		
		ambColor += lightsData[i].ambientColor;
	}
}

void main()
{
	vec3 normalTS = vec3(0, 0, 1);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	vec3 normal = -normalize(ts * normalTS);
	
	vec3 viewDir = normalize(psinput.worldPos.xyz - viewPosition);
	vec3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);
	
	vec3 reflectVec = reflect(viewDir, normal);
	vec3 envColor = textureCube(environmentMap, reflectVec).rgb;

	vec3 diffuse = envColor * (diffColor + ambColor);
	
	outputColor = vec4(diffuse, 1.0f);
}