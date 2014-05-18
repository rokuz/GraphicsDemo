#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 36) out;
out vec3 uv0;

uniform mat4 modelViewProjectionMatrix;

const vec4 cubeVerts[8] = 
{
	vec4(-0.5, -0.5, -0.5, 1),// LB  0
	vec4(-0.5, 0.5, -0.5, 1), // LT  1
	vec4(0.5, -0.5, -0.5, 1), // RB  2
	vec4(0.5, 0.5, -0.5, 1),  // RT  3
	vec4(-0.5, -0.5, 0.5, 1), // LB  4
	vec4(-0.5, 0.5, 0.5, 1),  // LT  5
	vec4(0.5, -0.5, 0.5, 1),  // RB  6
	vec4(0.5, 0.5, 0.5, 1)    // RT  7
};

const int cubeIndices[24] =
{
	0, 1, 2, 3, // front
	7, 6, 3, 2, // right
	7, 5, 6, 4, // back
	4, 0, 6, 2, // bottom
	1, 0, 5, 4, // left
	3, 1, 7, 5  // top
};

void main()
{
	vec4 v[8];
	
	for (int j = 0; j < 8; j++)
	{
		v[j] = modelViewProjectionMatrix * cubeVerts[j];
	}
	
	for (int i = 0; i < 6; i++)
	{
		int index = cubeIndices[i * 4];
		gl_Position = v[index];
		uv0 = cubeVerts[index].xyz;
		EmitVertex();

		index = cubeIndices[i * 4 + 2];
		gl_Position = v[index];
		uv0 = cubeVerts[index].xyz;
		EmitVertex();

		index = cubeIndices[i * 4 + 1];
		gl_Position = v[index];
		uv0 = cubeVerts[index].xyz;
		EmitVertex();
		EndPrimitive();

		index = cubeIndices[i * 4 + 1];
		gl_Position = v[index];
		uv0 = cubeVerts[index].xyz;
		EmitVertex();

		index = cubeIndices[i * 4 + 2];
		gl_Position = v[index];
		uv0 = cubeVerts[index].xyz;
		EmitVertex();

		index = cubeIndices[i * 4 + 3];
		gl_Position = v[index];
		uv0 = cubeVerts[index].xyz;
		EmitVertex();
		EndPrimitive();
	}
}