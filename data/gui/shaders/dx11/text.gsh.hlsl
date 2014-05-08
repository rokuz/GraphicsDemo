struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

static const VS_OUTPUT VERTS[4] =
{
	float4(-1, 1, 1, 1),
	float4(-1, -1, 1, 1),
	float4(1, -1, 1, 1),
	float4(1, 1, 1, 1)
};

[maxvertexcount(6)]
void main(point VS_OUTPUT pnt[1], inout TriangleStream<VS_OUTPUT> triStream )
{
	triStream.Append(VERTS[0]);
	triStream.Append(VERTS[1]);
	triStream.Append(VERTS[2]);
	triStream.RestartStrip();
	
	triStream.Append(VERTS[2]);
	triStream.Append(VERTS[3]);
	triStream.Append(VERTS[0]);
	triStream.RestartStrip();
}