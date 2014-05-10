/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once
#pragma warning(disable:4005 4996)
#ifdef _OS_WINDOWS7
#include <d3d11.h>
#elif _OS_WINDOWS8
#include <d3d11_1.h>
#elif _OS_WINDOWS81
#include <d3d11_2.h>
#endif

namespace framework
{

#ifdef _OS_WINDOWS7
	typedef ID3D11Device ID3D11Device_T;
	typedef ID3D11DeviceContext ID3D11DeviceContext_T;
	typedef IDXGISwapChain IDXGISwapChain_T;
	typedef IDXGIFactory IDXGIFactory_T;
	typedef IDXGIAdapter IDXGIAdapter_T;
	typedef DXGI_MODE_DESC DXGI_MODE_DESC_T;
	typedef DXGI_ADAPTER_DESC DXGI_ADAPTER_DESC_T;
	typedef IDXGIOutput IDXGIOutput_T;
	typedef DXGI_SWAP_CHAIN_DESC DXGI_SWAP_CHAIN_DESC_T;
#elif _OS_WINDOWS8
	typedef ID3D11Device1 ID3D11Device_T;
	typedef ID3D11DeviceContext1 ID3D11DeviceContext_T;
	typedef IDXGISwapChain1 IDXGISwapChain_T;
	typedef IDXGIFactory1 IDXGIFactory_T;
	typedef IDXGIAdapter1 IDXGIAdapter_T;
	typedef DXGI_MODE_DESC1 DXGI_MODE_DESC_T;
	typedef DXGI_ADAPTER_DESC1 DXGI_ADAPTER_DESC_T;
	typedef IDXGIOutput1 IDXGIOutput_T;
	typedef DXGI_SWAP_CHAIN_DESC DXGI_SWAP_CHAIN_DESC_T;
#elif _OS_WINDOWS81
	typedef ID3D11Device2 ID3D11Device_T;
	typedef ID3D11DeviceContext2 ID3D11DeviceContext_T;
	typedef IDXGISwapChain2 IDXGISwapChain_T;
	typedef IDXGIFactory2 IDXGIFactory_T;
	typedef IDXGIAdapter2 IDXGIAdapter_T;
	typedef DXGI_MODE_DESC1 DXGI_MODE_DESC_T;
	typedef DXGI_ADAPTER_DESC2 DXGI_ADAPTER_DESC_T;
	typedef IDXGIOutput2 IDXGIOutput_T;
	typedef DXGI_SWAP_CHAIN_DESC1 DXGI_SWAP_CHAIN_DESC_T;
#endif

struct Device
{
	ID3D11Device_T* device;
	ID3D11DeviceContext_T* context;
	IDXGISwapChain_T* swapChain;
	ID3D11Debug* debugger;
	D3D_FEATURE_LEVEL featureLevel;
	Device() : device(0), context(0), debugger(0), swapChain(0) {}
};

enum ShaderType
{
	VERTEX_SHADER = 0,
	HULL_SHADER,
	DOMAIN_SHADER,
	GEOMETRY_SHADER,
	PIXEL_SHADER,
	COMPUTE_SHADER,

	SHADERS_COUNT
};


class UtilitiesD3D11
{
public:
	static int sizeOfFormat( DXGI_FORMAT format )
	{
		switch( format )
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128 / 8;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96 / 8;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 64 / 8;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return 32 / 8;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return 16 / 8;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
			return 8 / 8;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
			return 128 / 8;

		case DXGI_FORMAT_R1_UNORM:
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 64 / 8;

		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			return 32 / 8;

		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			return 32 / 8;

		case DXGI_FORMAT_UNKNOWN:
		default:
			return 0;
		}
	}
};

}