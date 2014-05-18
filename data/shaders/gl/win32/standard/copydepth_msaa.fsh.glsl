#version 430 core

uniform sampler2DMS depthMap;
uniform int samplesCount;

void main()
{
	ivec2 coords = ivec2(gl_FragCoord.xy);
	float depth = 0;
	for(int i = 0; i < samplesCount; i++)
		depth += texelFetch(depthMap, coords, i).r;

	gl_FragDepth = depth / float(samplesCount);
}