#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

out VS_OUT
{
	vec2 uv0;
	vec3 ts0;
	vec3 ts1;
} vs_out;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(position, 1);
    vs_out.uv0 = uv0;
	
	mat3 modelMatrixRot = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
	vec3 normalWS = modelMatrixRot * normalize(normal);
	vec3 tangentWS = modelMatrixRot * normalize(tangent);
	vec3 binormalWS = modelMatrixRot * normalize(binormal);
	mat3 ts = inverse(mat3(tangentWS, binormalWS, normalWS));
	vs_out.ts0 = ts[0];
	vs_out.ts1 = ts[2];
}