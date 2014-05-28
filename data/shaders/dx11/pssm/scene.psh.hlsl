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
StructuredBuffer<LightData> lightsData : register(t0);

texture2D diffuseMap : register(t1);
texture2D normalMap : register(t2);
texture2D specularMap : register(t3);
SamplerState defaultSampler;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	const float specPower = 30.0;

	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	float ndol = max(0, dot(lightsData[0].direction, normal));
	
	float3 textureColor = diffuseMap.Sample(defaultSampler, input.uv0).rgb;
	float3 diffuse = textureColor * lightsData[0].diffuseColor * ndol;
	
	float3 h = normalize(input.viewDirection + lightsData[0].direction);
	float3 specularColor = specularMap.Sample(defaultSampler, input.uv0.xy).rgb;
	float3 specular = specularColor * lightsData[0].specularColor * pow (max(dot(normal, h), 0.0), specPower);
	
	float3 ambient = textureColor * lightsData[0].ambientColor;
	
    return float4(saturate(ambient + diffuse + specular), 1);
}