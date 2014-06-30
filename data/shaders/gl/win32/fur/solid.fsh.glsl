#version 430 core

in VS_OUTPUT
{
	vec2 uv0;
	vec3 normal;
	vec3 tangent;
	vec3 worldPos;
} psinput;

out vec4 outputColor;

// lights
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
layout(std430) buffer lightsDataBuffer
{
    LightData lightsData[];
};

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform vec3 viewPosition;

void main()
{
	const float SPEC_POWER = 10.0;

	vec3 normalTS = normalize(texture(normalMap, psinput.uv0).rgb * 2.0 - 1.0);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	vec3 normal = -normalize(ts * normalTS);

	float ndol = max(0, dot(lightsData[0].direction, normal));
	vec3 textureColor = texture(diffuseMap, psinput.uv0).rgb;
	vec3 diffuse = textureColor * lightsData[0].diffuseColor * ndol;
	
	vec3 viewDirection = normalize(psinput.worldPos - viewPosition);
	vec3 h = normalize(viewDirection + lightsData[0].direction);
	vec3 specColor = texture(specularMap, psinput.uv0).rgb;
	vec3 specular = specColor * lightsData[0].specularColor * pow(max(dot(normal, h), 0.0), SPEC_POWER);
	
	vec3 ambient = textureColor * lightsData[0].ambientColor;
	
    outputColor = vec4(clamp(ambient + diffuse + specular, 0, 1), 1);
}