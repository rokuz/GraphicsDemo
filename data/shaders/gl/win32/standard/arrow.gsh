#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 10) out;

uniform mat4 viewProjectionMatrix;
uniform vec4 orientation = vec4(0, 0, 0, 1);
uniform vec3 position = vec3(0, 0, 0);

vec3 z_direction(vec4 q)
{
	return vec3(2 * q.w * q.y + 2 * q.x * q.z,
				2 * q.y * q.z - 2 * q.x * q.w,
				q.w * q.w + q.z * q.z - q.x * q.x - q.y * q.y);
}

vec3 x_direction(vec4 q)
{
	return vec3(q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z,
				2 * q.w * q.z + 2 * q.y * q.x,
				2 * q.x * q.z - 2 * q.y * q.w);
}

vec3 y_direction(vec4 q)
{
	return vec3(2 * q.y * q.x - 2 * q.z * q.w,
				q.w * q.w + q.y * q.y - q.z * q.z - q.x * q.x,
				2 * q.z * q.y + 2 * q.x * q.w);
}

void main()
{
    gl_Position = viewProjectionMatrix * vec4(position, 1);
	EmitVertex();
	
	vec3 zdir = z_direction(orientation);
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 3.0, 1);
	EmitVertex();
	
	vec3 xdir = x_direction(orientation);
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 2.5 + xdir * 0.3, 1);
	EmitVertex();
	
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 2.75, 1);
	EmitVertex();
	
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 2.5 - xdir * 0.3, 1);
	EmitVertex();
	
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 3.0, 1);
	EmitVertex();
	
	vec3 ydir = y_direction(orientation);
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 2.5 + ydir * 0.3, 1);
	EmitVertex();
	
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 2.75, 1);
	EmitVertex();
	
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 2.5 - ydir * 0.3, 1);
	EmitVertex();
	
	gl_Position = viewProjectionMatrix * vec4(position + zdir * 3.0, 1);
	EmitVertex();
}