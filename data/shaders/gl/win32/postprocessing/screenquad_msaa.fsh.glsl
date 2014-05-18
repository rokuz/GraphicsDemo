#version 430 core

out vec4 outputColor;

uniform sampler2DMS screenquadMap;
uniform int samplesCount;

void main()
{
	ivec2 coords = ivec2(gl_FragCoord.xy);
	vec4 c = vec4(0);
	for(int i = 0; i < samplesCount; i++)
		c += texelFetch(screenquadMap, coords, i);

	outputColor = c / float(samplesCount);
}