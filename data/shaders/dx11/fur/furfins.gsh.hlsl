#include <common.h.hlsl>

struct GS_INPUT
{
    float4 position : SV_POSITION;
	float3 normal : TEXCOORD0;
};

struct GS_OUTPUT
{
    float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

[maxvertexcount(6)]
void main(lineadj GS_INPUT pnt[4], inout TriangleStream<GS_OUTPUT> triStream)
{
	float3 c1 = (pnt[0].position.xyz + pnt[1].position.xyz + pnt[2].position.xyz) / 3.0f;
	float3 c2 = (pnt[1].position.xyz + pnt[2].position.xyz + pnt[3].position.xyz) / 3.0f;
	float3 viewDirection1 = -normalize(viewPosition - c1);
	float3 viewDirection2 = -normalize(viewPosition - c2);
	float3 n1 = normalize(cross(pnt[0].position.xyz - pnt[1].position.xyz, pnt[2].position.xyz - pnt[1].position.xyz));
	float3 n2 = normalize(cross(pnt[1].position.xyz - pnt[2].position.xyz, pnt[3].position.xyz - pnt[2].position.xyz));
	float viewDotN1 = dot(n1, viewDirection1);
	float viewDotN2 = dot(n2, viewDirection2);

	if (viewDotN1 * viewDotN2 > 0)
	{
		GS_OUTPUT p[4];
		p[0].position = mul(pnt[1].position, modelViewProjection);
		p[0].uv0 = float2(0, 0);
		p[1].position = mul(pnt[2].position, modelViewProjection);
		p[1].uv0 = float2(0, 0);
		p[2].position = mul(float4(pnt[1].position.xyz + pnt[1].normal * 0.03, 1), modelViewProjection);
		p[2].uv0 = float2(0, 0);
		p[3].position = mul(float4(pnt[2].position.xyz + pnt[2].normal * 0.03, 1), modelViewProjection);
		p[3].uv0 = float2(0, 0);

		triStream.Append(p[2]);
		triStream.Append(p[1]);
		triStream.Append(p[0]);
		triStream.RestartStrip();
	
		triStream.Append(p[1]);
		triStream.Append(p[2]);
		triStream.Append(p[3]);
		triStream.RestartStrip();
	}
}