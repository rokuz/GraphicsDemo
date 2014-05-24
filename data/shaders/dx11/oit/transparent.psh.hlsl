#include <tpcommon.h.hlsl>

float4 main(VS_OUTPUT input) : SV_TARGET
{
	uint2 upos = uint2(input.position.xy);
	uint index = headBuffer[upos];
	clip(index == 0xffffffff ? -1 : 1);
	
	float3 color = float3(0, 0, 0);
	float alpha = 1;
	
	NodeData sortedFragments[MAX_FRAGMENTS];
	[unroll]
	for (int j = 0; j < MAX_FRAGMENTS; j++)
	{
		sortedFragments[j] = (NodeData)0;
	}

	int counter;
	insertionSort(index, sortedFragments, counter);
	for (int i = 0; i < counter; i++)
	{
		float4 c = unpackColor(sortedFragments[i].packedColor);
		alpha *= (1.0 - c.a);
		color = lerp(color, c.rgb, c.a);
	}

    return float4(color, alpha);
}