#version 430 core

const int MAX_SPLITS = 4;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUTPUT
{
	float depth;
	flat int instanceID;
} gsinput[];

out float outputDepth;

uniform int shadowIndices[MAX_SPLITS];

void main()
{
	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Position = gl_in[i].gl_Position;
		gl_Layer = shadowIndices[gsinput[i].instanceID];
		outputDepth = gsinput[i].depth;
		EmitVertex();
	}
	EndPrimitive();
}