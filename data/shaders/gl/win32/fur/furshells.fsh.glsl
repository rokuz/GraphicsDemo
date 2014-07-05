#version 430 core

in VS_OUTPUT
{
	vec3 uv0;
	vec3 normal;
	vec3 tangent;
	vec3 worldPos;
} psinput;

out vec4 outputColor;

const float FUR_LAYERS = 16.0f;
const float FUR_SELF_SHADOWING = 0.9f;
const float FUR_SCALE = 50.0f;
const float FUR_SPECULAR_POWER = 0.35f;

// lights
struct LightData
{
	vec3 position;
	uint lightType;
	vec3 direction;
	float falloff;
	vec3 diffuseColor;
	float angle;
	vec3 ambientColor;
	uint dummy;
	vec3 specularColor;
	uint dummy2;
};
layout(std430) buffer lightsDataBuffer
{
    LightData lightsData[];
};

uniform sampler2D diffuseMap;
uniform sampler2DArray furMap;
uniform vec3 viewPosition;

void main()
{
	const float specPower = 30.0;

	vec3 coords = psinput.uv0 * vec3(FUR_SCALE, FUR_SCALE, 1.0);
	vec4 fur = texture(furMap, coords);
	if (fur.a < 0.01) discard;

	float d = psinput.uv0.z / FUR_LAYERS;
	outputColor = vec4(texture(diffuseMap, psinput.uv0.xy).rgb, fur.a * (1.0 - d));

	vec3 viewDirection = normalize(psinput.worldPos - viewPosition);
	
	vec3 tangentVector = normalize((fur.rgb - 0.5) * 2.0);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	tangentVector = normalize(ts * tangentVector);

	float TdotL = dot(tangentVector, lightsData[0].direction);
	float TdotE = dot(tangentVector, viewDirection);
	float sinTL = sqrt(1 - TdotL * TdotL);
	float sinTE = sqrt(1 - TdotE * TdotE);
	outputColor.rgb = lightsData[0].ambientColor * outputColor.rgb +
					  lightsData[0].diffuseColor * (1.0 - sinTL) * outputColor.rgb +
					  lightsData[0].specularColor * pow(abs((TdotL * TdotE + sinTL * sinTE)), specPower) * FUR_SPECULAR_POWER;

	float shadow = d * (1.0 - FUR_SELF_SHADOWING) + FUR_SELF_SHADOWING;
	outputColor.rgb *= shadow;
}