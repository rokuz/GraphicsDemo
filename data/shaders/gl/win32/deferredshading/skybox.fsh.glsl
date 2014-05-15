#version 430 core

in vec3 uv0;
out vec4 outputColor;

uniform samplerCube skyboxMap;

void main()
{
	vec3 uv = normalize(uv0);
    outputColor = textureCube(skyboxMap, uv);
}