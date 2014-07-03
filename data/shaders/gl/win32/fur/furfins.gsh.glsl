#version 430 core

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 6) out;

in VS_OUTPUT
{
	vec2 uv0;
	vec3 normal;
} gsinput[];

out vec3 texcoords;

const float FUR_LAYERS = 16.0f;
const float FUR_LENGTH = 0.03f;

uniform mat4 modelViewProjectionMatrix;
uniform sampler2D furLengthMap;
uniform vec3 viewPosition;

void main()
{
	vec3 c1 = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0f;
	vec3 c2 = (gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz + gl_in[3].gl_Position.xyz) / 3.0f;
	vec3 viewDirection1 = -normalize(viewPosition - c1);
	vec3 viewDirection2 = -normalize(viewPosition - c2);
	vec3 n1 = normalize(cross(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz));
	vec3 n2 = normalize(cross(gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz, gl_in[3].gl_Position.xyz - gl_in[2].gl_Position.xyz));
	float edge = dot(n1, viewDirection1) * dot(n2, viewDirection2);

	float furLen = texture(furLengthMap, gsinput[1].uv0).r * FUR_LENGTH;
	
	vec4 p[4];
	vec3 uv[4];
	if (edge > 0 && furLen > 1e-3)
	{
		p[0] = modelViewProjectionMatrix * vec4(gl_in[1].gl_Position.xyz, 1);
		uv[0] = vec3(gsinput[1].uv0, 0);
		p[1] = modelViewProjectionMatrix * vec4(gl_in[2].gl_Position.xyz, 1);
		uv[1] = vec3(gsinput[2].uv0, 0);
		p[2] = modelViewProjectionMatrix * vec4(gl_in[1].gl_Position.xyz + gsinput[1].normal * furLen, 1);
		uv[2] = vec3(gsinput[1].uv0, FUR_LAYERS - 1);
		p[3] = modelViewProjectionMatrix * vec4(gl_in[2].gl_Position.xyz + gsinput[2].normal * furLen, 1);
		uv[3] = vec3(gsinput[2].uv0, FUR_LAYERS - 1);

		gl_Position = p[2]; texcoords = uv[2];
		EmitVertex();
		gl_Position = p[1]; texcoords = uv[1];
		EmitVertex();
		gl_Position = p[0]; texcoords = uv[0];
		EmitVertex();
		EndPrimitive();
	
		gl_Position = p[1]; texcoords = uv[1];
		EmitVertex();
		gl_Position = p[2]; texcoords = uv[2];
		EmitVertex();
		gl_Position = p[3]; texcoords = uv[3];
		EmitVertex();
		EndPrimitive();
	}
}