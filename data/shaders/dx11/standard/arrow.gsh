struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

cbuffer arrowData
{
	matrix modelViewProjection : packoffset(c0);
	float3 position : packoffset(c4);
	float4 orientation : packoffset(c5);
	float4 color : packoffset(c6);
};

float3 z_direction(float4 q)
{
	return float3(2 * q.w * q.y + 2 * q.x * q.z,
				  2 * q.y * q.z - 2 * q.x * q.w,
				  q.w * q.w + q.z * q.z - q.x * q.x - q.y * q.y);
}

float3 x_direction(float4 q)
{
	return float3(q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z,
				  2 * q.w * q.z + 2 * q.y * q.x,
				  2 * q.x * q.z - 2 * q.y * q.w);
}

float3 y_direction(float4 q)
{
	return float3(2 * q.y * q.x - 2 * q.z * q.w,
				  q.w * q.w + q.y * q.y - q.z * q.z - q.x * q.x,
				  2 * q.z * q.y + 2 * q.x * q.w);
}

[maxvertexcount(10)]
void main(point VS_OUTPUT pnt[1], inout LineStream<VS_OUTPUT> lineStream )
{
	VS_OUTPUT v1;
    v1.position = mul(modelViewProjection, float4(position, 1));
	lineStream.Append(v1);
	
	VS_OUTPUT v2;
	float3 zdir = z_direction(orientation);
	v2.position = mul(modelViewProjection, float4(position + zdir * 3.0, 1));
	lineStream.Append(v2);
	
	VS_OUTPUT v3;
	float3 xdir = x_direction(orientation);
	v3.position = mul(modelViewProjection, float4(position + zdir * 2.5 + xdir * 0.3, 1));
	lineStream.Append(v3);
	
	VS_OUTPUT v4;
	v4.position = mul(modelViewProjection, float4(position + zdir * 2.75, 1));
	lineStream.Append(v4);
	
	VS_OUTPUT v5;
	v5.position = mul(modelViewProjection, float4(position + zdir * 2.5 - xdir * 0.3, 1));
	lineStream.Append(v5);
	
	VS_OUTPUT v6;
	v6.position = mul(modelViewProjection, float4(position + zdir * 3.0, 1));
	lineStream.Append(v6);
	
	VS_OUTPUT v7;
	float3 ydir = y_direction(orientation);
	v7.position = mul(modelViewProjection, float4(position + zdir * 2.5 + ydir * 0.3, 1));
	lineStream.Append(v7);
	
	VS_OUTPUT v8;
	v8.position = mul(modelViewProjection, float4(position + zdir * 2.75, 1));
	lineStream.Append(v8);
	
	VS_OUTPUT v9;
	v9.position = mul(modelViewProjection, float4(position + zdir * 2.5 - ydir * 0.3, 1));
	lineStream.Append(v9);
	
	VS_OUTPUT v10;
	v10.position = mul(modelViewProjection, float4(position + zdir * 3.0, 1));
	lineStream.Append(v10);
}