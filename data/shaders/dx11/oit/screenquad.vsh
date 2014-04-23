struct VS_INPUT
{
    float3 position : POSITION;
};
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = float4(0,0,0,1);
    return output;
}