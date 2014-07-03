#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

out VS_OUTPUT
{
	vec3 uv0;
	vec3 normal;
	vec3 tangent;
	vec3 worldPos;
} vsoutput;

const float FUR_LAYERS = 16.0f;
const float FUR_LENGTH = 0.03f;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform sampler2D furLengthMap;

void main()
{
	float furLen = texture(furLengthMap, uv0).r;
	vec3 pos = position + normalize(normal) * furLen * FUR_LENGTH * float(gl_InstanceID + 1) / FUR_LAYERS;
	gl_Position = modelViewProjectionMatrix * vec4(pos, 1);
	vsoutput.uv0 = vec3(uv0, float(gl_InstanceID));
	mat3 modelMatrixRot = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
	vsoutput.normal = modelMatrixRot * normalize(normal);
	vsoutput.tangent = modelMatrixRot * normalize(tangent);
	vsoutput.worldPos = (modelMatrix * vec4(pos, 1)).xyz;
}