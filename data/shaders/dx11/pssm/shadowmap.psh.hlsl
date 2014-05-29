struct PS_INPUT
{
    float4 position : SV_POSITION;
	float depth : TEXCOORD0;
	uint index : SV_RenderTargetArrayIndex;
};

float main(PS_INPUT input) : SV_TARGET
{
    return input.depth;
}