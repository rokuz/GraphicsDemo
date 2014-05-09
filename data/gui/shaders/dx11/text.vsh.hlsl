struct VS_INPUT
{
    float3 position : POSITION;
};
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	unsigned int instanceID : SV_InstanceID;
};

VS_OUTPUT main(VS_INPUT input, unsigned int instanceID : SV_InstanceID)
{
    VS_OUTPUT output;
    output.position = float4(0, 0, 0, 1);
	output.instanceID = instanceID;
    return output;
}