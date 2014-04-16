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

#include "sampler.h"
#include "logger.h"
#include "outputd3d11.h"
#include "application.h"

namespace framework
{

const D3D11_SAMPLER_DESC& Sampler::getDefault()
{
	static D3D11_SAMPLER_DESC desc;
	static bool isInitialized = false;
	if (!isInitialized)
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.BorderColor[0] = 0.0f; 
		desc.BorderColor[1] = 0.0f; 
		desc.BorderColor[2] = 0.0f; 
		desc.BorderColor[3] = 0.0f;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
	}
	return desc;
}

Sampler::Sampler() :
	m_state(0)
{
	m_description = getDefault();
}

Sampler::~Sampler()
{
	destroy();
}

void Sampler::initWithDescription(const D3D11_SAMPLER_DESC& desc)
{
	const Device& device = Application::instance()->getDevice();

	m_description = desc;
	HRESULT hr = device.device->CreateSamplerState(&m_description, &m_state);
	if (hr != S_OK)
	{
		utils::Logger::toLogWithFormat("Error: could not initialize a sampler: %s\n", toString(m_description).c_str());
	}

	if (isValid()) initDestroyable();
}

void Sampler::destroy()
{
	if (m_state != 0)
	{
		m_state->Release();
		m_state = 0;
	}
}

bool Sampler::isValid() const
{
	return m_state != 0;
}

}