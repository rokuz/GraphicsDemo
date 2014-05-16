#version 430 core

in VS_OUTPUT_GBUF
{
	vec2 uv0;
	vec3 normal;
	vec3 tangent;
	vec3 worldViewPos;
} psinput;

layout(location = 0) out vec4 dataBlock1;
layout(location = 1) out uvec4 dataBlock2;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

uniform float specularPower;
uniform uint materialId;

uint packColor(vec4 color)
{
	return (uint(color.r * 255) << 24) | (uint(color.g * 255) << 16) | (uint(color.b * 255) << 8) | uint(color.a * 255);
}

vec2 packNormal(vec3 normal)
{
	return vec2(atan(normal.y, normal.x), normal.z);
}

void main()
{
	vec3 normalTS = normalize(texture(normalMap, psinput.uv0).rgb * 2.0 - 1.0);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	vec3 normal = -normalize(ts * normalTS);
	
	dataBlock1 = vec4(length(psinput.worldViewPos), packNormal(normal), specularPower);
	dataBlock2.x = (materialId & 0x0000ffff) | (gl_SampleMaskIn[0] << 16);
	dataBlock2.y = packColor(texture(diffuseMap, psinput.uv0));
	dataBlock2.z = packColor(texture(specularMap, psinput.uv0));
}