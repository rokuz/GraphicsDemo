#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

const vec4 VERTS[4] =
{
	vec4(-1, 1, 1, 1),
	vec4(-1, -1, 1, 1),
	vec4(1, -1, 1, 1),
	vec4(1, 1, 1, 1)
};

void main()
{
	gl_Position = VERTS[0];
	EmitVertex();

	gl_Position = VERTS[1];
	EmitVertex();

	gl_Position = VERTS[2];
	EmitVertex();
	EndPrimitive();

	gl_Position = VERTS[2];
	EmitVertex();

	gl_Position = VERTS[3];
	EmitVertex();

	gl_Position = VERTS[0];
	EmitVertex();
	EndPrimitive();
}