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

#include "renderTarget.h"
#include "logger.h"
#include "application.h"

namespace framework
{

D3D11_TEXTURE2D_DESC RenderTarget::getDefaultDesc(int width, int height, DXGI_FORMAT format)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	return desc;
}

D3D11_TEXTURE2D_DESC RenderTarget::getDefaultDepthDesc(int width, int height)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	return desc;
}

RenderTarget::RenderTarget() :
	m_colorBuffer(0),
	m_depthBuffer(0),
	m_useDepth(false),
	m_isSwapChain(false)
{

}

RenderTarget::~RenderTarget()
{
	destroy();
}

void RenderTarget::initWithSwapChain(const Device& device, bool withDepth)
{
	destroy();

	// always use depth in this case
	m_useDepth = withDepth;
	m_isSwapChain = true;

	// get buffer from swap chain
	m_colorBuffer.resize(1);
	m_colorBufferDesc.resize(1);
	HRESULT hr = device.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_colorBuffer[0]));
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not get buffer from swap chain.\n");
		return;
	}
	m_colorBuffer[0]->GetDesc(&m_colorBufferDesc[0]);

	// initialize view
	m_view.resize(1);
	if (!createView(0)) return;

	// depth buffer
	if (m_useDepth)
	{
		if (!createDepth(0)) return;
	}

	if (isValid()) initDestroyable();
}

void RenderTarget::initWithDescription(const D3D11_TEXTURE2D_DESC& desc, bool withDepth)
{
	const Device& device = Application::instance()->getDevice();

	destroy();

	m_colorBuffer.resize(1);
	m_colorBufferDesc.resize(1);
	m_useDepth = withDepth;
	m_colorBufferDesc[0] = desc;
	m_isSwapChain = false;

	// create a render target
	HRESULT hr = device.device->CreateTexture2D(&m_colorBufferDesc[0], 0, &m_colorBuffer[0]);
	if (hr != S_OK)
	{
		destroy();
		utils::Logger::toLog("Error: could not create a render target.\n");
		return;
	}

	// initialize view
	m_view.resize(1);
	if (!createView(0)) return;

	// depth buffer
	if (m_useDepth)
	{
		if (!createDepth(0)) return;
	}

	if (isValid()) initDestroyable();
}

void RenderTarget::initWithDescriptions( const std::vector<D3D11_TEXTURE2D_DESC>& descs, bool withDepth )
{
	const Device& device = Application::instance()->getDevice();

	destroy();

	m_colorBuffer.resize(descs.size());
	m_view.resize(descs.size());
	m_useDepth = withDepth;
	m_colorBufferDesc = descs;
	m_isSwapChain = false;

	for (size_t i = 0; i < descs.size(); i++)
	{
		// create a render target
		HRESULT hr = device.device->CreateTexture2D(&m_colorBufferDesc[i], 0, &m_colorBuffer[i]);
		if (hr != S_OK)
		{
			destroy();
			utils::Logger::toLogWithFormat("Error: could not create a render target (%d).\n", i);
			return;
		}

		// initialize view
		if (!createView(i)) return;
	}

	// depth buffer
	if (m_useDepth)
	{
		if (!createDepth(0)) return;
	}

	if (isValid()) initDestroyable();
}

bool RenderTarget::createDepth(size_t index)
{
	const Device& device = Application::instance()->getDevice();

	m_depthBufferDesc = getDefaultDepthDesc(m_colorBufferDesc[index].Width, m_colorBufferDesc[index].Height);
	m_depthBufferDesc.SampleDesc.Count = m_colorBufferDesc[index].SampleDesc.Count;
	m_depthBufferDesc.SampleDesc.Quality = m_colorBufferDesc[index].SampleDesc.Quality;

	HRESULT hr = device.device->CreateTexture2D(&m_depthBufferDesc, 0, &m_depthBuffer);
	if (hr != S_OK)
	{
		destroy();
		utils::Logger::toLog("Error: could not create a depth buffer.\n");
		return false;
	}

	// initialize depth view
	m_depthView.init(device, m_depthBuffer, m_depthBufferDesc.BindFlags);
	if (!m_depthView.isValid())
	{
		destroy();
		return false;
	}

	return true;
}

bool RenderTarget::createView(size_t index)
{
	const Device& device = Application::instance()->getDevice();

	m_view[index].init(device, m_colorBuffer[index], m_colorBufferDesc[index].BindFlags);
	if (!m_view[index].isValid())
	{
		destroy();
		return false;
	}

	return true;
}

bool RenderTarget::isValid() const
{
	if (m_colorBuffer.empty()) return false;
	for (size_t i = 0; i < m_colorBuffer.size(); i++)
	{
		if (m_colorBuffer[i] == 0) return false;
	}

	return (m_useDepth ? m_depthBuffer != 0 : true);
}

const ResourceView& RenderTarget::getView(int index) const
{
	return m_view[index];
}

int RenderTarget::getViewCount() const
{
	return (int)m_view.size();
}

bool RenderTarget::isDepthUsed() const
{
	return m_useDepth;
}

const ResourceView& RenderTarget::getDepthView() const
{
	return m_depthView;
}

const D3D11_TEXTURE2D_DESC& RenderTarget::getDesc(int index) const
{
	return m_colorBufferDesc[index];
}

ID3D11Texture2D* RenderTarget::getTexture( int index ) const
{
	return m_colorBuffer[index];
}

void RenderTarget::destroy()
{
	for (size_t i = 0; i < m_view.size(); i++)
	{
		if (m_colorBuffer[i] != 0) { m_colorBuffer[i]->Release(); }
		m_view[i].destroy();
	}
	m_view.clear();
	m_colorBuffer.clear();
	m_colorBufferDesc.clear();
	
	m_isSwapChain = false;

	m_useDepth = false;
	m_depthView.destroy();
	if (m_depthBuffer != 0)
	{
		m_depthBuffer->Release();
		m_depthBuffer = 0;
	}
}

}