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

#include "depthstencilstage.h"
#include "logger.h"
#include "outputd3d11.h"
#include <algorithm>
#undef min
#undef max

namespace framework
{

const D3D11_DEPTH_STENCIL_DESC& DepthStencilStage::getDefault()
{
	static D3D11_DEPTH_STENCIL_DESC desc;
	static bool isInitialized = false;
	if (!isInitialized)
	{
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

		desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		isInitialized = true;
	}
	return desc;
}

DepthStencilStage::DepthStencilStage() :
	m_state(0),
	m_stencilRef(0)
{
	m_description = getDefault();
}

DepthStencilStage::~DepthStencilStage()
{
	destroy();
}

void DepthStencilStage::initWithDescription(const D3D11_DEPTH_STENCIL_DESC& desc)
{
	m_description = desc;
	PipelineStage::init();
}

void DepthStencilStage::destroy()
{
	if (m_state != 0)
	{
		m_state->Release();
		m_state = 0;
	}
}

bool DepthStencilStage::isValid()
{
	return m_state != 0;
}

void DepthStencilStage::onInit(const Device& device)
{
	HRESULT hr = device.device->CreateDepthStencilState(&m_description, &m_state);
	if (hr != S_OK)
	{
		utils::Logger::toLogWithFormat("Error: could not initialize a depth stencil stage: %s\n", toString(m_description).c_str());
	}
}

void DepthStencilStage::onApply(const Device& device)
{
	if (m_state != 0)
	{
		device.context->OMSetDepthStencilState(m_state, m_stencilRef);
	}
}

}