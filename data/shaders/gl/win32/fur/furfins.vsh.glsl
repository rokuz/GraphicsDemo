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
} vsoutput;

void main()
{
	gl_Position = vec4(position, 1);
	vsoutput.uv0 = uv0;
	vsoutput.normal = normalize(normal);
}