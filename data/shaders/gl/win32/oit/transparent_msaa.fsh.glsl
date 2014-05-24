#version 430 core

out vec4 outputColor;

layout(r32ui) coherent uniform uimage2D headBuffer;

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};
layout(std430) buffer fragmentsList
{
	ListNode fragments[];
};

struct NodeData
{
	uint packedColor;
	float depth;
};

const int MAX_FRAGMENTS = 16;

vec4 unpackColor(uint color)
{
	vec4 c;
	c.r = float((color >> 24) & 0x000000ff) / 255.0f;
	c.g = float((color >> 16) & 0x000000ff) / 255.0f;
	c.b = float((color >> 8) & 0x000000ff) / 255.0f;
	c.a = float(color & 0x000000ff) / 255.0f;
	return clamp(c, 0.0f, 1.0f);
}

void insertionSort(uint startIndex, int sampleIndex, inout NodeData sortedFragments[MAX_FRAGMENTS], out int counter)
{
	counter = 0;
	uint index = startIndex;
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		if (index != 0xffffffff)
		{
			uint coverage = (fragments[index].depthAndCoverage >> 16);
			if (coverage & (1 << sampleIndex))
			{
				sortedFragments[counter].packedColor = fragments[index].packedColor;
				sortedFragments[counter].depth = unpackHalf2x16(fragments[index].depthAndCoverage).x;
				counter++;
			}
			index = fragments[index].next;			
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

void main()
{
	ivec2 upos = ivec2(gl_FragCoord.xy);
	uint index = imageLoad(headBuffer, upos).x;
	if (index == 0xffffffff) discard;

	vec3 color = vec3(0);
	float alpha = 1.0f;

	NodeData sortedFragments[MAX_FRAGMENTS];
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		sortedFragments[i] = NodeData(0, 0.0f);
	}

	int counter;
	insertionSort(index, gl_SampleID, sortedFragments, counter);

	// resolve multisampling
	int resolveBuffer[MAX_FRAGMENTS];
	vec4 colors[MAX_FRAGMENTS];
	int resolveIndex = -1;
	float prevdepth = -1.0f;
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
	for (int i = 0; i < counter; i++)
	{
		if (resolveBuffer[i] != 0)
		{
			vec4 c = colors[i] / float(resolveBuffer[i]);
			alpha *= (1.0 - c.a);
			color = mix(color, c.rgb, c.a);
		}
	}
	
	/*for (int i = 0; i < counter; i++)
	{
		vec4 c = unpackColor(sortedFragments[i].packedColor);
		alpha *= (1.0 - c.a);
		color = mix(color, c.rgb, c.a);
	}*/

    outputColor = vec4(color, alpha);
}