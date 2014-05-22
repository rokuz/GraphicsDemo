#version 430 core

layout(r32ui) writeonly uniform uimage2D headBuffer;

void main()
{
	ivec2 upos = ivec2(gl_FragCoord.xy);
	uvec4 data = uvec4(0xffffffff);
	imageStore(headBuffer, upos, data);
}