#include <common.h.hlsl>

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float depth : TEXCOORD0;
	uint instanceID : SV_InstanceID;
};

VS_OUTPUT main(VS_INPUT input, unsigned int instanceID : SV_InstanceID)
{
	VS_OUTPUT output;
	float4 pos = mul(float4(input.position, 1), model);
    output.position = mul(pos, shadowViewProjection[shadowIndices[instanceID]]);
    output.depth = output.position.z;
	output.instanceID = instanceID;
	
	return output;
}