#version 430 core

in vec3 viewRay;

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
layout(std140) buffer lightsDataBuffer
{
    LightData lightsData[];
};

uniform sampler2D dataBlockMap1;
uniform usampler2D dataBlockMap2;
uniform mat4 viewInverseMatrix;
uniform vec3 viewPosition;
uniform uint lightsCount;

void processLight(const uint lightIndex, vec3 worldPos, out vec3 lightDir, out float lightPower)
{
	if (lightsData[lightIndex].lightType == 0) // omni light
	{
		vec3 ldv = worldPos - lightsData[lightIndex].position;
		lightDir = normalize(ldv);
		float falloff = 1.0 - clamp(length(ldv) / (lightsData[lightIndex].falloff + 1e-7), 0.0f, 1.0f);
		lightPower = falloff * falloff;
	}
	else if (lightsData[lightIndex].lightType == 1) // spot light
	{
		vec3 ldv = worldPos - lightsData[lightIndex].position;
		lightDir = normalize(ldv);
		
		float cosAng = dot(lightDir, lightsData[lightIndex].direction);
		float cosOuter = cos(lightsData[lightIndex].angle);
		lightPower = smoothstep(cosOuter, cosOuter + 0.1, cosAng);
		
		if (lightPower > 1e-5)
		{
			float falloff = 1.0 - clamp(length(ldv) / (lightsData[lightIndex].falloff + 1e-7), 0.0f, 1.0f);
			lightPower *= (falloff * falloff);
		}
	}
	else // direct light
	{
		lightDir = lightsData[lightIndex].direction;
		lightPower = 1;
	}
}

void blinn(in vec3 normal, in vec3 worldPos, in float specPower, out vec3 diffColor, out vec3 specColor, out vec3 ambColor)
{
	vec3 viewDir = normalize(worldPos - viewPosition);

	diffColor = vec3(0, 0, 0);
	specColor = vec3(0, 0, 0);
	ambColor = vec3(0, 0, 0);
	
	for (uint i = 0; i < lightsCount; i++)
	{
		vec3 lightDir;
		float lightPower;
		processLight(i, worldPos, lightDir, lightPower);
		
		// skip weak lights
		if (lightPower > 0.01)
		{
			float ndol = max(0.0, dot(lightDir, normal));
			diffColor += lightsData[i].diffuseColor * ndol * lightPower;
			
			vec3 h = normalize(viewDir + lightDir);
			specColor += lightsData[i].specularColor * pow(max(dot(normal, h), 0.0), specPower) * lightPower;
			
			ambColor += lightsData[i].ambientColor * lightPower;
		}
	}
}

vec4 unpackColor(uint color)
{
	vec4 outcolor;
	outcolor.r = float((color >> 24) & 0x000000ff) / 255.0f;
	outcolor.g = float((color >> 16) & 0x000000ff) / 255.0f;
	outcolor.b = float((color >> 8) & 0x000000ff) / 255.0f;
	outcolor.a = float(color & 0x000000ff) / 255.0f;
	return clamp(outcolor, 0.0f, 1.0f);
}

vec3 unpackNormal(vec2 normal)
{
	vec2 theta = vec2(sin(normal.x), cos(normal.x));
	vec2 phi = vec2(sqrt(1.0f - normal.y * normal.y), normal.y);
	return vec3(theta.y * phi.x, theta.x * phi.x, phi.y);
}

vec4 unpackPosition(vec3 viewRay, float l)
{
	return vec4(viewRay * l, 1);
}

void main()
{
	ivec2 coords = ivec2(gl_FragCoord.xy);
	
	// clip empty fragments
	uvec4 dataBlock2 = texelFetch(dataBlockMap2, coords, 0);
	uint matId = dataBlock2.x & 0x0000ffff;
	if (matId < 1) discard;
	
	vec3 nvr = normalize(viewRay);

	vec4 dataBlock1 = texelFetch(dataBlockMap1, coords, 0);
	vec3 worldPos = (viewInverseMatrix * unpackPosition(nvr, dataBlock1.x)).xyz;
	vec3 normal = unpackNormal(dataBlock1.yz);
	
	vec4 diffTex = unpackColor(dataBlock2.y);
	vec4 specularTex = unpackColor(dataBlock2.z);
	float specularPower = dataBlock1.w;
	
	// lighting
	vec3 diffColor, specColor, ambColor;
	blinn(normal, worldPos, specularPower, diffColor, specColor, ambColor);
	
	vec3 diffuse = diffTex.rgb * diffColor;
	vec3 specular = specularTex.rgb * specColor;
	vec3 ambient = diffTex.rgb * ambColor;
	
	outputColor = vec4(clamp(ambient + diffuse + specular, 0.0f, 1.0f), diffTex.a);
}