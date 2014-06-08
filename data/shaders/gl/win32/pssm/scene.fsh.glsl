#version 430 core

const int MAX_SPLITS = 4;
const float SHADOW_BIASES[MAX_SPLITS] = 
{ 
	0.0009, 0.002, 0.0025, 0.003
};

in VS_OUTPUT
{
	vec2 uv0;
	vec3 normal;
	vec3 tangent;
	vec3 worldPos;
} psinput;

out vec4 outputColor;

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

uniform int splitsCount;
uniform mat4 shadowViewProjection[MAX_SPLITS];
uniform vec3 viewPosition;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2DArrayShadow shadowMap;

vec3 getShadowCoords(int splitIndex, vec3 worldPos)
{
	vec4 coords = shadowViewProjection[splitIndex] * vec4(worldPos, 1);
	coords.xy = (coords.xy / coords.ww) * vec2(0.5, 0.5) + vec2(0.5, 0.5);
	return coords.xyz;
}

float sampleShadowMap(int index, vec3 coords, float bias) 
{ 
	float receiver = coords.z - bias;
	vec4 uv = vec4(coords.xy, receiver, float(index));
	return texture(shadowMap, uv); 

	/*float sum = 0.0;
	const float step = 1.0 / 1024.0;
	const int FILTER_SIZE = 3;
	for (int i = 0; i < FILTER_SIZE; i++)
	{
		for (int j = 0; j < FILTER_SIZE; j++)
		{
			sum += texture(shadowMap, uv + step * vec4(i, j, 0, 0)).r;
		}	
	}
	return sum / (FILTER_SIZE * FILTER_SIZE);*/
}

float shadow(vec3 worldPos)
{
	vec3 coords = getShadowCoords(0, worldPos);
	return sampleShadowMap(0, coords, 1.5);

	/*float shadowValue = 0;

	for (int i = 0; i < MAX_SPLITS; i++)
	{
		if (i < splitsCount)
		{
			vec3 coords = getShadowCoords(i, worldPos);
			shadowValue += (1.0 - sampleShadowMap(i, coords, SHADOW_BIASES[i]));
		}
	}
		
	return 1.0 - clamp(shadowValue, 0.0, 1.0);*/
}

void main()
{
	const float SPEC_POWER = 30.0;

	vec3 normalTS = normalize(texture(normalMap, psinput.uv0).rgb * 2.0 - 1.0);
	mat3 ts = mat3(psinput.tangent, cross(psinput.normal, psinput.tangent), psinput.normal);
	vec3 normal = -normalize(ts * normalTS);

	float ndol = max(0, dot(lightsData[0].direction, normal));

	// a kind of elimination of double shading
	float shadowValue = 1;
	//if (ndol > 0.1)
	{
		shadowValue = shadow(psinput.worldPos);
	}
	outputColor = vec4(shadowValue, shadowValue, shadowValue, 1);
		
	/*const float SHADOW_INTENSITY = 0.7;
	vec3 textureColor = texture(diffuseMap, psinput.uv0).rgb;
	textureColor = mix(textureColor, textureColor * shadowValue, SHADOW_INTENSITY);
	vec3 diffuse = textureColor * lightsData[0].diffuseColor * ndol;
	
	vec3 viewDirection = normalize(psinput.worldPos - viewPosition);
	vec3 h = normalize(viewDirection + lightsData[0].direction);
	vec3 specular = lightsData[0].specularColor * pow(max(dot(normal, h), 0.0), SPEC_POWER) * shadowValue;
	
	vec3 ambient = textureColor * lightsData[0].ambientColor;
	
    outputColor = vec4(clamp(ambient + diffuse + specular, 0, 1), 1);*/
}