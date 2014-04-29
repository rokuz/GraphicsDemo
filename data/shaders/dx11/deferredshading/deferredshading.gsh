#include <common.h>

struct GS_INPUT
{
    float4 position : SV_POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float3 viewRay : TEXCOORD0;
};

static const float4 VERTS[4] =
{
	float4(-1, 1, 1, 1),
	float4(-1, -1, 1, 1),
	float4(1, -1, 1, 1),
	float4(1, 1, 1, 1)
};

[maxvertexcount(6)]
void main(point GS_INPUT pnt[1], inout TriangleStream<VS_OUTPUT> triStream )
{
	VS_OUTPUT output[4];
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		output[i].position = VERTS[i];
		output[i].viewRay = mul(VERTS[i], projectionInverse).xyz;
	}
	
	triStream.Append(output[0]);
	triStream.Append(output[1]);
	triStream.Append(output[2]);
	triStream.RestartStrip();
	
	triStream.Append(output[2]);
	triStream.Append(output[3]);
	triStream.Append(output[0]);
	triStream.RestartStrip();
}