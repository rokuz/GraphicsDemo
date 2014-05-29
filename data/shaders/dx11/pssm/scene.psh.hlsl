struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 viewDirection : TEXCOORD3;
};

struct LightData
{
	float3 position;
	uint lightType;
	float3 direction;
	float falloff;
	float3 diffuseColor;
	float angle;
	float3 ambientColor;
	uint dummy;
	float3 specularColor;
	uint dummy2;
};
cbuffer lightsData : register(b2)
{
	LightData light;
};

texture2D diffuseMap : register(t1);
texture2D normalMap : register(t2);
SamplerState defaultSampler;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	const float specPower = 30.0;

	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	float ndol = max(0, dot(light.direction, normal));
	
	float3 textureColor = diffuseMap.Sample(defaultSampler, input.uv0).rgb;
	float3 diffuse = textureColor * light.diffuseColor * ndol;
	
	float3 h = normalize(input.viewDirection + light.direction);
	float3 specular = light.specularColor * pow(max(dot(normal, h), 0.0), specPower);
	
	float3 ambient = textureColor * light.ambientColor;
	
    return float4(saturate(ambient + diffuse + specular), 1);
}