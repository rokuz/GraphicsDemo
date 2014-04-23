struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};
RWTexture2D<uint> headBuffer;
RWStructuredBuffer<ListNode> fragmentsList;

struct NodeData
{
	uint packedColor;
	float depth;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

static const int MAX_FRAGMENTS = 16;
static const NodeData INITIAL_SORTED_FRAGMENTS[MAX_FRAGMENTS] = 
{ 
	(NodeData)0, (NodeData)0, (NodeData)0, (NodeData)0,
	(NodeData)0, (NodeData)0, (NodeData)0, (NodeData)0,
	(NodeData)0, (NodeData)0, (NodeData)0, (NodeData)0,
	(NodeData)0, (NodeData)0, (NodeData)0, (NodeData)0
};

float4 unpackColor(uint color)
{
	float4 output;
	output.r = float((color >> 24) & 0x000000ff) / 255.0f;
	output.g = float((color >> 16) & 0x000000ff) / 255.0f;
	output.b = float((color >> 8) & 0x000000ff) / 255.0f;
	output.a = float(color & 0x000000ff) / 255.0f;
	return saturate(output);
}

void insertionSort(uint startIndex, uint sampleIndex, inout NodeData sortedFragments[MAX_FRAGMENTS], out int counter)
{
	counter = 0;
	uint index = startIndex;
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		if (index != 0xffffffff)
		{
			uint coverage = (fragmentsList[index].depthAndCoverage >> 16);
			if (coverage & (1 << sampleIndex))
			{
				sortedFragments[counter].packedColor = fragmentsList[index].packedColor;
				sortedFragments[counter].depth = f16tof32(fragmentsList[index].depthAndCoverage);
				counter++;
			}
			index = fragmentsList[index].next;			
		}
	}
	
    for (int k = 1; k < MAX_FRAGMENTS; k++) 
	{
        int j = k;
        NodeData t = sortedFragments[k];
		
        while (sortedFragments[j - 1].depth < t.depth) 
		{
            sortedFragments[j] = sortedFragments[j - 1];
            j--;
            if (j <= 0) { break; }
        }
		
		if (j != k) { sortedFragments[j] = t; }
    }
}

float4 main(VS_OUTPUT input, uint sampleIndex : SV_SAMPLEINDEX) : SV_TARGET
{
	uint2 upos = uint2(input.position.xy);
	uint index = headBuffer[upos];
	
	float3 color = float3(0, 0, 0);
	float alpha = 1;
	
	NodeData sortedFragments[MAX_FRAGMENTS] = INITIAL_SORTED_FRAGMENTS;
	int counter;
	insertionSort(index, sampleIndex, sortedFragments, counter);
	for (int i = 0; i < counter; i++)
	{
		float4 c = unpackColor(sortedFragments[i].packedColor);
		alpha *= (1.0 - c.a);
		color = lerp(color, c.rgb, c.a);
	}

    return float4(color, alpha);
}