#version 430 core

in VS_OUT
{
	vec2 uv0;
	vec3 ts0;
	vec3 ts1;
	
} fs_in;

out vec4 color;

uniform sampler2D diffuseSampler;
uniform sampler2D normalSampler;
uniform sampler2D specularSampler;
uniform vec3 viewDirection;

struct LightData
{
	vec3 positionOrDirection;
	uint lightType;
	vec3 diffuseColor;
	float falloff;
	vec3 ambientColor;
	float angle;
	vec3 specularColor;
	uint dummy;
};

layout(std140) uniform lightsDataBuffer
{
    LightData light[16];
};


void main()
{
	vec3 normalTS = normalize(texture(normalSampler, fs_in.uv0).rgb * 2.0 - 1.0);
	mat3 ts = mat3(fs_in.ts0, cross(fs_in.ts1, fs_in.ts0), fs_in.ts1);
	vec3 lightDirTS = ts * light[0].positionOrDirection;
	vec3 viewDirTS = ts * viewDirection;
	float ndol = max(0,dot(-lightDirTS, normalTS));
	
	vec3 textureColor = texture(diffuseSampler, fs_in.uv0).rgb;
	vec3 diffuse = textureColor * ndol;
	
	vec3 reflectTS = reflect(lightDirTS, normalTS);
	vec3 specular = pow(max(dot(reflectTS, -viewDirTS), 0), 16) * texture(specularSampler, fs_in.uv0).rgb * ndol;
	
	vec3 ambient = vec3(0.3, 0.3, 0.3) * textureColor;
	
    color = vec4(ambient + diffuse + specular, 1); //vec4(texture(diffuseSampler, fs_in.uv0).rgb, 1);//
}