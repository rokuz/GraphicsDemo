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

#include "blendstage.h"
#include "logger.h"
#include "outputd3d11.h"
#include <algorithm>
#include "utils.h"
#undef min
#undef max

namespace framework
{

const D3D11_BLEND_DESC& BlendStage::getDefault()
{
	static D3D11_BLEND_DESC desc;
	static bool isInitialized = false;
	if (!isInitialized)
	{
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;
		for (int i = 0; i < 8; i++)
		{
			desc.RenderTarget[i].BlendEnable = false;
			desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			desc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
			desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		isInitialized = true;
	}
	return desc;
}

D3D11_BLEND_DESC BlendStage::getDisableColorWriting()
{
	D3D11_BLEND_DESC desc = getDefault();
	for (int i = 0; i < 8; i++)
	{
		desc.RenderTarget[i].RenderTargetWriteMask = 0;
	}
	return std::move(desc);
}

BlendStage::BlendStage() :
	m_state(0),
	m_blendFactor(1, 1, 1, 1),
	m_sampleMask(0xFFFFFFFF)
{
	m_description = getDefault();
}

BlendStage::~BlendStage()
{
	destroy();
}

void BlendStage::initWithDescription(const D3D11_BLEND_DESC& desc)
{
	m_description = desc;
	PipelineStage::init();
}

void BlendStage::destroy()
{
	if (m_state != 0)
	{
		m_state->Release();
		m_state = 0;
	}
}

bool BlendStage::isValid()
{
	return m_state != 0;
}

void BlendStage::onInit(const Device& device)
{
	HRESULT hr = device.device->CreateBlendState(&m_description, &m_state);
	if (hr != S_OK)
	{
		utils::Logger::toLogWithFormat("Error: could not initialize a blend stage: %s\n", toString(m_description).c_str());
	}
}

void BlendStage::onApply(const Device& device)
{
	if (m_state != 0)
	{
		device.context->OMSetBlendState(m_state, utils::Utils::convert(m_blendFactor), m_sampleMask);
	}
}

}