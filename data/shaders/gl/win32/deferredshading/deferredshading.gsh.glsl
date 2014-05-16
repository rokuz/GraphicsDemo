#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;
out vec3 viewRay;

uniform mat4 projectionInverseMatrix;

const vec4 VERTS[4] =
{
	vec4(-1, 1, 1, 1),
	vec4(-1, -1, 1, 1),
	vec4(1, -1, 1, 1),
	vec4(1, 1, 1, 1)
};

void main()
{
	vec3 viewRays[4];
	for (int i = 0; i < 4; i++)
	{
		viewRays[i] = (projectionInverseMatrix * VERTS[i]).xyz;
	}
	
	gl_Position = VERTS[0]; viewRay = viewRays[0];
	EmitVertex();

	gl_Position = VERTS[1]; viewRay = viewRays[1];
	EmitVertex();

	gl_Position = VERTS[2]; viewRay = viewRays[2];
	EmitVertex();
	EndPrimitive();

	gl_Position = VERTS[2]; viewRay = viewRays[2];
	EmitVertex();

	gl_Position = VERTS[3]; viewRay = viewRays[3];
	EmitVertex();

	gl_Position = VERTS[0]; viewRay = viewRays[0];
	EmitVertex();
	EndPrimitive();
}