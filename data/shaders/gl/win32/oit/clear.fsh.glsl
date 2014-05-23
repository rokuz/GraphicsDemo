#version 430 core

layout(binding = 0, r32ui) coherent uniform uimage2D headBuffer;

void main()
{
	ivec2 upos = ivec2(gl_FragCoord.xy);
	imageStore(headBuffer, upos, uvec4(0xffffffff));
}