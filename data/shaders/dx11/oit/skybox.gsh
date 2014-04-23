#include <common.h>

struct GS_INPUT
{
    float4 position : SV_POSITION;
};

struct VS_OUTPUT_SKYBOX
{
    float4 position : SV_POSITION;
	float3 uv : TEXCOORD0;
};

static const float4 cubeVerts[8] = 
{
	float4(-0.5, -0.5, -0.5, 1),// LB  0
	float4(-0.5, 0.5, -0.5, 1), // LT  1
	float4(0.5, -0.5, -0.5, 1), // RB  2
	float4(0.5, 0.5, -0.5, 1),  // RT  3
	float4(-0.5, -0.5, 0.5, 1), // LB  4
	float4(-0.5, 0.5, 0.5, 1),  // LT  5
	float4(0.5, -0.5, 0.5, 1),  // RB  6
	float4(0.5, 0.5, 0.5, 1)    // RT  7
};

static const int cubeIndices[24] =
{
	0, 1, 2, 3, // front
	7, 6, 3, 2, // right
	7, 5, 6, 4, // back
	4, 0, 6, 2, // bottom
	1, 0, 5, 4, // left
	3, 1, 7, 5  // top
};

[maxvertexcount(36)]
void main(point GS_INPUT pnt[1], inout TriangleStream<VS_OUTPUT_SKYBOX> triStream )
{
	VS_OUTPUT_SKYBOX v[8];
	[unroll]
	for (int j = 0; j < 8; j++)
	{
		v[j].position = mul(cubeVerts[j], modelViewProjection);
		v[j].uv = cubeVerts[j].xyz;
	}
	
	[unroll]
	for (int i = 0; i < 6; i++)
	{
		triStream.Append(v[cubeIndices[i * 4]]);
		triStream.Append(v[cubeIndices[i * 4 + 2]]);
		triStream.Append(v[cubeIndices[i * 4 + 1]]);
		triStream.RestartStrip();
		
		triStream.Append(v[cubeIndices[i * 4 + 1]]);
		triStream.Append(v[cubeIndices[i * 4 + 2]]);
		triStream.Append(v[cubeIndices[i * 4 + 3]]);
		triStream.RestartStrip();
	}
}