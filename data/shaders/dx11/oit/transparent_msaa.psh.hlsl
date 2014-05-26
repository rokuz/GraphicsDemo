#include <tpcommon.h.hlsl>

float4 main(VS_OUTPUT input, uint sampleIndex : SV_SAMPLEINDEX) : SV_TARGET
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
	insertionSortMSAA(index, sampleIndex, sortedFragments, counter);

	// resolve multisampling
	int resolveBuffer[MAX_FRAGMENTS];
	float4 colors[MAX_FRAGMENTS];
	int resolveIndex = -1;
	float prevdepth = -1.0f;
	[unroll(MAX_FRAGMENTS)]
	for (int i = 0; i < counter; i++)
	{
		if (sortedFragments[i].depth != prevdepth)
		{
			resolveIndex = -1;
			resolveBuffer[i] = 1;
			colors[i] = unpackColor(sortedFragments[i].packedColor);
		}
		else
		{
			if (resolveIndex < 0) { resolveIndex = i - 1; }

			colors[resolveIndex] += unpackColor(sortedFragments[i].packedColor);
			resolveBuffer[resolveIndex]++;

			resolveBuffer[i] = 0;
		}
		prevdepth = sortedFragments[i].depth;
	}

	// gather
	[unroll(MAX_FRAGMENTS)]
	for (int i = 0; i < counter; i++)
	{
		[branch]
		if (resolveBuffer[i] != 0)
		{
			float4 c = colors[i] / float(resolveBuffer[i]);
			alpha *= (1.0 - c.a);
			color = lerp(color, c.rgb, c.a);
		}
	}

    return float4(color, alpha);
}