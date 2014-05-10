#version 430 core

in VS_OUT
{
	vec2 uv0;
	vec3 ts0;
	vec3 ts1;
	vec3 viewDir;
} fs_in;

out vec4 color;

uniform sampler2D diffuseSampler;
uniform sampler2D normalSampler;
uniform sampler2D specularSampler;

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

layout(std140) uniform lightsDataBuffer
{
    LightData light[16];
};

void main()
{
	const float specPower = 30.0;

	vec3 normalTS = -normalize(texture(normalSampler, fs_in.uv0).rgb * 2.0 - 1.0);
	mat3 ts = mat3(fs_in.ts0, cross(fs_in.ts1, fs_in.ts0), fs_in.ts1);
	vec3 lightDirTS = ts * light[0].direction;
	vec3 viewDirTS = ts * fs_in.viewDir;
	float ndol = max(0, dot(lightDirTS, normalTS));
	
	vec3 diffColor = texture(diffuseSampler, fs_in.uv0).rgb;
	vec3 diffuse = diffColor * light[0].diffuseColor * ndol;
	
	vec3 h = normalize(viewDirTS + lightDirTS);
	vec3 specular = light[0].specularColor * texture(specularSampler, fs_in.uv0).rgb * pow(max(dot(normalTS, h), 0), specPower);
	
	vec3 ambient = diffColor * light[0].ambientColor;
	
    color = vec4(ambient + diffuse + specular, 1);
}