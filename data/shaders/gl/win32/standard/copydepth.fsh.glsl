#version 430 core

uniform sampler2D depthMap;

void main()
{
	ivec2 coords = ivec2(gl_FragCoord.xy);
	float depth = texelFetch(depthMap, coords, 0).r;
	gl_FragDepth = depth;
}