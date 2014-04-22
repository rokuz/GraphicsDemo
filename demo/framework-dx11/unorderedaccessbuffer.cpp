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

#include "unorderedaccessbuffer.h"
#include "logger.h"
#include "application.h"

namespace framework
{

D3D11_BUFFER_DESC UnorderedAccessBuffer::getDefaultUnorderedAcces(unsigned int size, unsigned int structsize)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = size * structsize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = structsize;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	return desc;
}

UnorderedAccessBuffer::UnorderedAccessBuffer() :
	m_uavFlags(0),
	m_stagingBuffer(0)
{
	m_checkSizeOnSet = false;
}

UnorderedAccessBuffer::~UnorderedAccessBuffer()
{
	destroy();
}

void UnorderedAccessBuffer::destroy()
{
	if (m_stagingBuffer != 0)
	{
		m_stagingBuffer->Release();
		m_stagingBuffer = 0;
	}

	UniformBuffer::destroy();
}

bool UnorderedAccessBuffer::initUnorderedAccess(size_t count, size_t structSize, const D3D11_BUFFER_DESC& desc, unsigned int flags)
{
	const Device& device = Application::instance()->getDevice();

	destroy();
	m_desc = desc;
	m_uavFlags = flags;

	HRESULT hr = device.device->CreateBuffer(&m_desc, 0, &m_buffer);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create an unordered access buffer.\n");
		return false;
	}

	if (isStructured())
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC svdesc = ResourceView::getDefaultShaderDesc();
		svdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		svdesc.Buffer.FirstElement = 0;
		svdesc.Buffer.NumElements = count;
		m_view.setShaderDesc(svdesc);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavdesc = ResourceView::getDefaultUAVDesc();
		uavdesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavdesc.Buffer.FirstElement = 0;
		uavdesc.Buffer.Flags = m_uavFlags;
		uavdesc.Buffer.NumElements = count;
		m_view.setUnorderedAccessDesc(uavdesc);

		m_view.init(device, m_buffer, m_desc.BindFlags);
	}

	D3D11_BUFFER_DESC stagingDesc;
	stagingDesc.ByteWidth = 4;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = device.device->CreateBuffer(&stagingDesc, nullptr, &m_stagingBuffer);
	if (hr != S_OK)
	{
		destroy();
		utils::Logger::toLog("Error: could not create an unordered access buffer, staging buffer creation failed.\n");
		return false;
	}

	if (m_buffer != 0) initDestroyable();
	return m_buffer != 0;
}

unsigned int UnorderedAccessBuffer::getActualSize()
{
	const Device& device = Application::instance()->getDevice();

	device.context->CopyStructureCount(m_stagingBuffer, 0, m_view.asUAView());
	D3D11_MAPPED_SUBRESOURCE subresource;
	device.context->Map(m_stagingBuffer, 0, D3D11_MAP_READ, 0, &subresource);
	unsigned int numActiveElements = *(unsigned int*)subresource.pData;
	device.context->Unmap(m_stagingBuffer, 0);

	return numActiveElements;
}

}