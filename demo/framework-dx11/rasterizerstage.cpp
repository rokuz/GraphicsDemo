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

#include "rasterizerstage.h"
#include "logger.h"
#include "outputd3d11.h"
#include <algorithm>
#undef min
#undef max

namespace framework
{

const D3D11_RASTERIZER_DESC& RasterizerStage::getDefault()
{
	static D3D11_RASTERIZER_DESC desc;
	static bool isInitialized = false;
	if (!isInitialized)
	{
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = false;
		desc.DepthBias = 0;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthBiasClamp = 0.0f;
		desc.DepthClipEnable = true;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = false;
		desc.AntialiasedLineEnable = true;
		isInitialized = true;
	}
	return desc;
}

D3D11_VIEWPORT RasterizerStage::getDefaultViewport(int width, int height)
{
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	return viewport;
}

D3D11_RECT RasterizerStage::getDefaultScissorRect(int x, int y, int width, int height)
{
	D3D11_RECT rect;
	rect.left = (LONG)x;
	rect.top = (LONG)y;
	rect.right = rect.left + (LONG)width;
	rect.bottom = rect.top + (LONG)height;
	return rect;
}

RasterizerStage::RasterizerStage():
	m_state(0)
{
	m_viewports.reserve(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_scissorRects.reserve(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_description = getDefault();
}

RasterizerStage::~RasterizerStage()
{

}

void RasterizerStage::initWithDescription(const Device& device, const D3D11_RASTERIZER_DESC& desc)
{
	m_description = desc;
	PipelineStage::init(device);
}

void RasterizerStage::destroy()
{
	if (m_state != 0)
	{
		m_state->Release();
		m_state = 0;
	}
}

bool RasterizerStage::isValid()
{
	return m_state != 0;
}

void RasterizerStage::onInit(const Device& device)
{
	HRESULT hr = device.device->CreateRasterizerState(&m_description, &m_state);
	if (hr != S_OK)
	{
		utils::Logger::toLogWithFormat("Error: could not initialize a rasterizer stage: %s\n", toString(m_description).c_str());
	}
}

void RasterizerStage::onApply(const Device& device)
{
	if (m_state != 0)
	{
		device.context->RSSetState(m_state);
	}
	if (!m_viewports.empty())
	{
		unsigned int sz = std::min((unsigned int)m_viewports.size(), (unsigned int)D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
		device.context->RSSetViewports(sz, m_viewports.data());
	}
	if (!m_scissorRects.empty())
	{
		unsigned int sz = std::min((unsigned int)m_scissorRects.size(), (unsigned int)D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
		device.context->RSSetScissorRects(sz, m_scissorRects.data());
	}
}

void RasterizerStage::addViewport(const D3D11_VIEWPORT& viewport)
{
	m_viewports.push_back(viewport);
}

void RasterizerStage::clearViewports()
{
	m_viewports.clear();
}

int RasterizerStage::getViewportsCount() const
{
	return (int)m_viewports.size();
}

void RasterizerStage::addScissorRect(const D3D11_RECT& rect)
{
	m_scissorRects.push_back(rect);
}

void RasterizerStage::clearScissorRects()
{
	m_scissorRects.clear();
}

int RasterizerStage::getScissorRectsCount() const
{
	return (int)m_scissorRects.size();
}

}