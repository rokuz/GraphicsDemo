#include <common.h.hlsl>
#include <dscommon.h.hlsl>

Texture2DMS<float4> dataBlockMap1 : register(t1);
Texture2DMS<uint3> dataBlockMap2 : register(t2);

PS_OUTPUT_DS main(PS_INPUT_DS input, uint sampleIndex : SV_SAMPLEINDEX)
{
	PS_OUTPUT_DS output;
	
	// clip empty fragments
	int sumMatId = 0;
	for (uint i = 0; i < samplesCount; i++)
	{
		uint3 dataBlock2 = dataBlockMap2.Load(int2(input.position.xy), i);
		sumMatId += (dataBlock2.x & 0x0000ffff);
	}
	clip(sumMatId - 1);
	
	float3 nvr = normalize(input.viewRay);
	
	// resolving MSAA & deferred lighting
	float4 result = 0;
	float numSamples = 0;
	uint fullCoverageMask = (1 << samplesCount) - 1;
	for (int sample = 0; sample < samplesCount; sample++)
	{
		uint3 dataBlock2 = dataBlockMap2.Load(int2(input.position.xy), sample);
		uint coverage = (dataBlock2.x >> 16);
		
		[branch]
		if (coverage & (1 << sampleIndex))
		{
			float4 dataBlock1 = dataBlockMap1.Load(int2(input.position.xy), sample);
			float3 worldPos = mul(unpackPosition(nvr, dataBlock1.x), viewInverse).xyz;
			float3 normal = unpackNormal(dataBlock1.yz);
			
			float4 diffTex = unpackColor(dataBlock2.y);
			float4 specularTex = unpackColor(dataBlock2.z);
			float specularPower = dataBlock1.w;
			
			// lighting
			float3 diffColor, specColor, ambColor;
			blinn(normal, worldPos, specularPower, diffColor, specColor, ambColor);
			
			float3 diffuse = diffTex * diffColor;
			float3 specular = specularTex.rgb * specColor;
			float3 ambient = diffTex * ambColor;
			
			result += float4(saturate(ambient + diffuse + specular), diffTex.a);
		
			numSamples++;
		}
		
		// full covered fragment we will lit once
		if (coverage == fullCoverageMask) break;
	}

	output.color = result / numSamples;
    return output;
}