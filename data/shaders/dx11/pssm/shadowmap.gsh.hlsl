struct VS_INPUT
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
void main(triangle VS_INPUT pnt[3], inout TriangleStream<GS_OUTPUT> triStream)
{
	triStream.Append((GS_OUTPUT)pnt[0]);
	triStream.Append((GS_OUTPUT)pnt[1]);
	triStream.Append((GS_OUTPUT)pnt[2]);
	triStream.RestartStrip();
}