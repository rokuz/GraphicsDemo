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
layout(std140) buffer lightsDataBuffer
{
    LightData lightsData[];
};

uniform samplerCube environmentMap;
uniform vec3 viewPosition;
uniform uint lightsCount;

layout(r32ui) coherent uniform uimage2D headBuffer;

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};
layout(std430) buffer fragmentsList
{
	ListNode fragments[];
};

layout(binding = 2, offset = 0) uniform atomic_uint fragmentsListCounter;

uint packColor(vec4 color)
{
	return (uint(color.r * 255) << 24) | (uint(color.g * 255) << 16) | (uint(color.b * 255) << 8) | uint(color.a * 255);
}

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

vec4 computeColorTransparent(bool frontFace)
{
	vec3 normalTS = vec3(0, 0, 1);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	vec3 normal = normalize(ts * normalTS) * (frontFace ? -1.0f : 1.0f);;
	
	vec3 viewDir = normalize(psinput.worldPos.xyz - viewPosition);
	vec3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);
	
	vec3 reflectVec = reflect(viewDir, normal);
	vec3 envColor = textureCube(environmentMap, reflectVec).rgb;
	float alpha = clamp(1.0f - dot(viewDir, normal), 0.3f, 1.0f);
	vec3 diffuse = envColor * (diffColor + ambColor);
	
	return vec4(diffuse, alpha);
}

void main()
{
	uint newHeadBufferValue = atomicCounterIncrement(fragmentsListCounter);
	if (newHeadBufferValue == 0xffffffff) discard;

	vec4 color = computeColorTransparent(gl_FrontFacing);
	
	ivec2 upos = ivec2(gl_FragCoord.xy);
	uint previosHeadBufferValue = imageAtomicExchange(headBuffer, upos, newHeadBufferValue);
	
	uint currentDepth = packHalf2x16(vec2(psinput.worldPos.w, 0));
	fragments[newHeadBufferValue].packedColor = packColor(vec4(color.rgb, color.a));
	fragments[newHeadBufferValue].depthAndCoverage = currentDepth | (gl_SampleMaskIn[0] << 16);
	fragments[newHeadBufferValue].next = previosHeadBufferValue;
}