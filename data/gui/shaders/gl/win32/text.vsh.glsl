#version 430 core

out int instanceID;

void main()
{
    gl_Position = vec4(0, 0, 0, 1);
	instanceID = gl_InstanceID;
}