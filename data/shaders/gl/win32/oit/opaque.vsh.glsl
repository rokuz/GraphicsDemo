#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

out VS_OUTPUT
{
	vec2 uv0;
	vec3 normal;
	vec3 tangent;
	vec4 worldPos;
} vsoutput;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

void main()
{
	vec4 pos = modelViewProjectionMatrix * vec4(position, 1);
	gl_Position = pos;
	mat3 modelMatrixRot = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
    vsoutput.uv0 = uv0;	
	vsoutput.normal = modelMatrixRot * normalize(normal);
	vsoutput.tangent = modelMatrixRot * normalize(tangent);
	vsoutput.worldPos = vec4((modelMatrix * vec4(position, 1)).xyz, pos.z);
}