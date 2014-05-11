#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
in flat int instanceID[];
out vec2 uv[];

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

	gl_Position = vec4((character.rectangle.xy - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uv[0] = character.uv * textureSize;
	EmitVertex();

	gl_Position = vec4((character.rectangle.xw - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uv[1] = (character.uv + vec2(0, height)) * textureSize;
	EmitVertex();

	gl_Position = vec4((character.rectangle.zw - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uv[2] = (character.uv + vec2(width, height)) * textureSize;
	EmitVertex();

	gl_Position = vec4((character.rectangle.zy - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	uv[3] = (character.uv + vec2(width, 0)) * textureSize;
	EmitVertex();
}