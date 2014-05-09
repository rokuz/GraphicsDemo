struct GS_INPUT
{
	float4 position : SV_POSITION;
	unsigned int instanceID : SV_InstanceID;
};

struct GS_OUTPUT
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

struct CharacterData
{
	float4 rectangle;
	float2 uv;
};
StructuredBuffer<CharacterData> charactersData : register(t0);

[maxvertexcount(6)]
void main(point GS_INPUT pnt[1], inout TriangleStream<GS_OUTPUT> triStream)
{
	CharacterData character = charactersData[pnt[0].instanceID];
	float width = character.rectangle.z - character.rectangle.x;
	float height = character.rectangle.w - character.rectangle.y;
	GS_OUTPUT v[4];
	v[0].position = float4((character.rectangle.xy - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	v[0].uv0 = character.uv * textureSize;
	
	v[1].position = float4((character.rectangle.xw - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	v[1].uv0 = (character.uv + float2(0, height)) * textureSize;
	
	v[2].position = float4((character.rectangle.zw - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	v[2].uv0 = (character.uv + float2(width, height)) * textureSize;
	
	v[3].position = float4((character.rectangle.zy - halfScreenSize.xy) * halfScreenSize.zw, 1, 1);
	v[3].uv0 = (character.uv + float2(width, 0)) * textureSize;

	triStream.Append(v[0]);
	triStream.Append(v[1]);
	triStream.Append(v[2]);
	triStream.RestartStrip();
	
	triStream.Append(v[2]);
	triStream.Append(v[3]);
	triStream.Append(v[0]);
	triStream.RestartStrip();
}