struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

[maxvertexcount(6)]
void main(point VS_OUTPUT pnt[1], inout TriangleStream<VS_OUTPUT> triStream )
{
	VS_OUTPUT v[4];
	v[0].position = float4(-1, 1, 1, 1);
	v[1].position = float4(-1, -1, 1, 1);
	v[2].position = float4(1, -1, 1, 1);
	v[3].position = float4(1, 1, 1, 1);
	
	triStream.Append(v[0]);
	triStream.Append(v[1]);
	triStream.Append(v[2]);
	triStream.RestartStrip();
	
	triStream.Append(v[2]);
	triStream.Append(v[3]);
	triStream.Append(v[0]);
	triStream.RestartStrip();
}