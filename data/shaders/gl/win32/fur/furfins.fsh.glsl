#version 430 core

in vec3 texcoords;
out vec4 outputColor;

const float FUR_LAYERS = 16.0f;
const float FUR_SELF_SHADOWING = 0.9f;
const float FUR_SCALE = 50.0f;

uniform sampler2D diffuseMap;
uniform sampler2DArray furMap;

void main()
{
	outputColor.rgb = texture(diffuseMap, texcoords.xy).rgb;
	vec3 coords = texcoords * vec3(FUR_SCALE, FUR_SCALE, 1.0);
	vec4 fur = texture(furMap, coords);
	if (fur.a < 0.01) discard;

	float d = texcoords.z / (FUR_LAYERS - 1.0);
	outputColor.a = fur.a * (1.0 - d);

	float shadow = d * (1.0 - FUR_SELF_SHADOWING) + FUR_SELF_SHADOWING;
	outputColor.rgb *= shadow;
}