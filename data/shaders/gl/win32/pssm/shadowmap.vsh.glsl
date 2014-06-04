#version 430 core

const int MAX_SPLITS = 4;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

out VS_OUTPUT
{
	float depth;
	flat int instanceID;
} vsoutput;

uniform mat4 modelMatrix;
uniform mat4 shadowViewProjection[MAX_SPLITS];
uniform int shadowIndices[MAX_SPLITS];

void main()
{
	vec4 wpos = modelMatrix * vec4(position, 1);
	vec4 pos = shadowViewProjection[shadowIndices[gl_InstanceID]] * vec4(wpos.xyz, 1);
    
	gl_Position = pos;
    vsoutput.depth = pos.z;
	vsoutput.instanceID = gl_InstanceID;
}