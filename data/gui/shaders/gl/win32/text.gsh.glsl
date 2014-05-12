#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;
in flat int instanceID[];
out vec2 uv0;

uniform vec4 halfScreenSize;
uniform vec2 textureSize;

struct CharacterData
{
	vec4 rectangle;
	vec2 uv;
	uint dummy[2];
};
layout(std140) uniform charactersData
{
    CharacterData characters[256];
};

void main()
{
	CharacterData character = characters[instanceID[0]];
	float width = character.rectangle.z - character.rectangle.x;
	float height = character.rectangle.w - character.rectangle.y;

	vec4 positions[4];
	vec2 uvs[4];
	positions[0] = vec4((character.rectangle.xy - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uvs[0] = character.uv * textureSize;
	positions[1] = vec4((character.rectangle.xw - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uvs[1] = (character.uv + vec2(0, height)) * textureSize;
	positions[2] = vec4((character.rectangle.zw - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uvs[2] = (character.uv + vec2(width, height)) * textureSize;
	positions[3] = vec4((character.rectangle.zy - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uvs[3] = (character.uv + vec2(width, 0)) * textureSize;

	gl_Position = positions[0]; uv0 = uvs[0];
	EmitVertex();

	gl_Position = positions[1]; uv0 = uvs[1];
	EmitVertex();

	gl_Position = positions[2]; uv0 = uvs[2];
	EmitVertex();
	EndPrimitive();

	gl_Position = positions[2]; uv0 = uvs[2];
	EmitVertex();

	gl_Position = positions[3]; uv0 = uvs[3];
	EmitVertex();

	gl_Position = positions[0]; uv0 = uvs[0];
	EmitVertex();
}