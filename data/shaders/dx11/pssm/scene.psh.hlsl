struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float3 tangent : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float3 worldPos : TEXCOORD3;
};

cbuffer onFrameData : register(b1)
{
	float3 viewPosition;
	int splitsCount;
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

static const int MAX_SPLITS = 8;
cbuffer shadowData : register(b3)
{
	matrix shadowViewProjection[MAX_SPLITS];
};

texture2D diffuseMap : register(t1);
texture2D normalMap : register(t2);
SamplerState defaultSampler : register(s0);

Texture2DArray<float> shadowMap : register(t3);
SamplerComparisonState shadowMapSampler : register(s1);

float3 getShadowCoords(int splitIndex, float3 worldPos)
{
	float4 coords = mul(float4(worldPos, 1), shadowViewProjection[splitIndex]);
	coords.xy = (coords.xy / coords.ww) * float2(0.5, -0.5) + float2(0.5, 0.5);
	return coords.xyz;
}

float sampleShadowMap(int index, float3 coords, float bias) 
{ 
	float3 uv = float3(coords.xy, index); 
	float receiver = coords.z;
	return shadowMap.SampleCmpLevelZero(shadowMapSampler, uv, receiver - bias); 

	/*float sum = 0.0;
	const float step = 1.0 / 1024.0;
	
	for (int i = 0; i <= filterSize; i++)
	{
		for (int j = 0; j <= filterSize; j++)
		{
			sum += sm.SampleCmpLevelZero(ssShadowMap, uv + step * float2(i,j), zReceiver - bias); 
		}	
	}
	float sz = ((float)filterSize + 1.0);
	
	return sum / (sz * sz);*/
}

float shadow(float3 worldPos)
{
	float shadowValue = 0;

	[unroll(MAX_SPLITS)]
	for (int i = 0; i < splitsCount; i++)
	{
		float3 coords = getShadowCoords(i, worldPos);
		shadowValue += (1.0 - sampleShadowMap(i, coords, 0.001));
	}
		
	return 1.0 - saturate(shadowValue);
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	const float specPower = 30.0;

	float3 normalTS = normalize(normalMap.Sample(defaultSampler, input.uv0).rgb * 2.0 - 1.0);
	float3x3 ts = float3x3(input.tangent, cross(input.normal, input.tangent), input.normal);
	float3 normal = -normalize(mul(normalTS, ts));
	float ndol = max(0, dot(light.direction, normal));

	float shadowValue = shadow(input.worldPos);
	//ndol = min(ndol, shadowValue);
	//float3 textureColor = diffuseMap.Sample(defaultSampler, input.uv0).rgb;
	//return float4(textureColor * shadowValue, 1);
	
	float3 textureColor = diffuseMap.Sample(defaultSampler, input.uv0).rgb * shadowValue;
	float3 diffuse = textureColor * light.diffuseColor * ndol;
	
	float3 viewDirection = normalize(input.worldPos - viewPosition);
	float3 h = normalize(viewDirection + light.direction);
	float3 specular = light.specularColor * pow(max(dot(normal, h), 0.0), specPower);
	
	float3 ambient = textureColor * light.ambientColor;
	
    return float4(saturate(ambient + diffuse + specular), 1);
}