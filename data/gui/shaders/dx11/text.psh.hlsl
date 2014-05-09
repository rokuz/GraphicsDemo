struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

cbuffer textData : register(b0)
{
	float4 textColor;
	float4 area;
	float4 halfScreenSize;
	float2 textureSize;
	uint2 dummy;
};

texture2D charactersMap : register(t1);
SamplerState defaultSampler;

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 pos = input.position.xy;
	bool clipTest = pos.x < area.x || pos.x > area.z || pos.y < area.y || pos.y > area.w;
	clip(clipTest ? -1 : 1);

	float color = charactersMap.Sample(defaultSampler, input.uv0);
	return textColor * color;
}