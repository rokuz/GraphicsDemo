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

#include "stdafx.h"
#include "resourceview.h"

namespace framework
{

D3D11_SHADER_RESOURCE_VIEW_DESC ResourceView::getDefaultShaderDesc()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_SRV_DIMENSION_UNKNOWN;
	return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC ResourceView::getDefaultRenderTargetDesc()
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_RTV_DIMENSION_UNKNOWN;
	return desc;
}

D3D11_DEPTH_STENCIL_VIEW_DESC ResourceView::getDefaultDepthStencilDesc()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Flags = 0;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_DSV_DIMENSION_UNKNOWN;
	return desc;
}

D3D11_UNORDERED_ACCESS_VIEW_DESC ResourceView::getDefaultUAVDesc()
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_UAV_DIMENSION_UNKNOWN;
	return desc;
}

D3D11_SHADER_RESOURCE_VIEW_DESC ResourceView::getTexture2DShaderDesc( int arraySize, bool msaa )
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = arraySize > 1 ? (msaa ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY) : 
										 (msaa ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D);
	if (arraySize > 1)
	{
		if (msaa)
		{
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
		}
		else
		{
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.MipLevels = -1;
			desc.Texture2DArray.MostDetailedMip = 0;
		}
	}
	else
	{
		if (!msaa)
		{
			desc.Texture2D.MipLevels = -1;
			desc.Texture2D.MostDetailedMip = 0;
		}
	}
	return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC ResourceView::getTexture2DRenderTargetDesc( int arraySize, bool msaa )
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = arraySize > 1 ? (msaa ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY) : 
										 (msaa ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D);
	if (arraySize > 1)
	{
		if (msaa)
		{
			desc.Texture2DArray.MipSlice = 0;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
		}
		else
		{
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.MipSlice = 0;
		}
	}
	else
	{
		if (!msaa)
		{
			desc.Texture2D.MipSlice = 0;
		}
	}
	return desc;
}

D3D11_DEPTH_STENCIL_VIEW_DESC ResourceView::getTexture2DDepthStencilDesc( int arraySize, bool msaa )
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Flags = 0;
	desc.ViewDimension = arraySize > 1 ? (msaa ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY) : 
										 (msaa ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
	if (arraySize > 1)
	{
		if (msaa)
		{
			desc.Texture2DArray.MipSlice = 0;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
		}
		else
		{
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.MipSlice = 0;
		}
	}
	else
	{
		if (!msaa)
		{
			desc.Texture2D.MipSlice = 0;
		}
	}
	return desc;
}

D3D11_UNORDERED_ACCESS_VIEW_DESC ResourceView::getTexture2DUAVDesc( int arraySize )
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = arraySize > 1 ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;
	if (arraySize > 1)
	{
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = arraySize;
	}
	else
	{
		desc.Texture2D.MipSlice = 0;
	}
	return desc;
}

ResourceView::ResourceView() :
	m_shaderDesc(std::make_pair(D3D11_SHADER_RESOURCE_VIEW_DESC(), false)),
	m_renderTargetDesc(std::make_pair(D3D11_RENDER_TARGET_VIEW_DESC(), false)),
	m_depthStencilDesc(std::make_pair(D3D11_DEPTH_STENCIL_VIEW_DESC(), false)),
	m_uavDesc(std::make_pair(D3D11_UNORDERED_ACCESS_VIEW_DESC(), false)),
	m_shaderView(0),
	m_renderTargetView(0),
	m_depthStencilView(0),
	m_uaView(0)
{

}

ResourceView::~ResourceView()
{
	destroy();
}

void ResourceView::setShaderDesc(const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
	m_shaderDesc.first = desc;
	m_shaderDesc.second = true;
}
void ResourceView::setRenderTargetDesc(const D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
	m_renderTargetDesc.first = desc;
	m_renderTargetDesc.second = true;
}
void ResourceView::setDepthStencilDesc(const D3D11_DEPTH_STENCIL_VIEW_DESC& desc)
{
	m_depthStencilDesc.first = desc;
	m_depthStencilDesc.second = true;
}

void ResourceView::setUnorderedAccessDesc(const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	m_uavDesc.first = desc;
	m_uavDesc.second = true;
}

void ResourceView::init(const Device& device, ID3D11Resource* resource, unsigned int bindFlags)
{
	destroy();

	if ((bindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC* desc = m_shaderDesc.second ? &m_shaderDesc.first : nullptr;
		HRESULT hr = device.device->CreateShaderResourceView(resource, desc, &m_shaderView);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: could not create shader resource view.\n");
		}
		m_shaderDesc.second = true;
	}
	else
	{
		m_shaderDesc.second = false;
	}

	if ((bindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC* desc = m_renderTargetDesc.second ? &m_renderTargetDesc.first : nullptr;
		HRESULT hr = device.device->CreateRenderTargetView(resource, desc, &m_renderTargetView);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: could not create render target view.\n");
		}
		m_renderTargetDesc.second = true;
	}
	else
	{
		m_renderTargetDesc.second = false;
	}

	if ((bindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC* desc = m_depthStencilDesc.second ? &m_depthStencilDesc.first : nullptr;
		HRESULT hr = device.device->CreateDepthStencilView(resource, desc, &m_depthStencilView);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: could not create depth stencil view.\n");
		}
		m_depthStencilDesc.second = true;
	}
	else
	{
		m_depthStencilDesc.second = false;
	}

	if ((bindFlags & D3D11_BIND_UNORDERED_ACCESS) == D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC* desc = m_uavDesc.second ? &m_uavDesc.first : nullptr;
		HRESULT hr = device.device->CreateUnorderedAccessView(resource, desc, &m_uaView);
		if (hr != S_OK)
		{
			utils::Logger::toLog("Error: could not create unordered access view.\n");
		}
		m_uavDesc.second = true;
	}
	else
	{
		m_uavDesc.second = false;
	}
}

bool ResourceView::isValid() const
{
	if (m_shaderDesc.second && !m_shaderView) return false;
	if (m_renderTargetDesc.second && !m_renderTargetView) return false;
	if (m_depthStencilDesc.second && !m_depthStencilView) return false;
	if (m_uavDesc.second && !m_uaView) return false;
	return true;
}

void ResourceView::destroy()
{
	if (m_shaderView != 0)
	{
		m_shaderView->Release();
		m_shaderView = 0;
		m_shaderDesc.second = false;
	}

	if (m_renderTargetView != 0)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
		m_renderTargetDesc.second = false;
	}

	if (m_depthStencilView != 0)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
		m_depthStencilDesc.second = false;
	}

	if (m_uaView != 0)
	{
		m_uaView->Release();
		m_uaView = 0;
		m_uavDesc.second = false;
	}
}

}