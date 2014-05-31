#include <common.h.hlsl>

struct GS_INPUT
{
    float4 position : SV_POSITION;
	float depth : TEXCOORD0;
	uint instanceID : SV_InstanceID;
};

struct GS_OUTPUT
{
    float4 position : SV_POSITION;
	float depth : TEXCOORD0;
	uint index : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void main(triangle GS_INPUT pnt[3], inout TriangleStream<GS_OUTPUT> triStream)
{
	GS_OUTPUT p = (GS_OUTPUT)pnt[0];
	p.index = shadowIndices[pnt[0].instanceID];
	triStream.Append(p);

	p = (GS_OUTPUT)pnt[1];
	p.index = shadowIndices[pnt[1].instanceID];
	triStream.Append(p);

	p = (GS_OUTPUT)pnt[2];
	p.index = shadowIndices[pnt[2].instanceID];
	triStream.Append(p);
	triStream.RestartStrip();
}