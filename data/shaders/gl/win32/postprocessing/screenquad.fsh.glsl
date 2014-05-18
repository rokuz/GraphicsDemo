#version 430 core

out vec4 outputColor;

uniform sampler2D screenquadMap;

void main()
{
	ivec2 coords = ivec2(gl_FragCoord.xy);
	vec4 dataBlock2 = texelFetch(screenquadMap, coords, 0);
	outputColor = dataBlock2;
}