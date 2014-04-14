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

#include "line3D.h"
#include "gpuprogram.h"
#include "standardGpuPrograms.h"
#include "uniformBuffer.h"
#include "geometry3D.h"

namespace framework
{

Line3D::Line3D() :
	m_vertexBuffer(0),
	m_isInitialized(false)
{
}

Line3D::~Line3D()
{
	destroy();
}

bool Line3D::initWithArray(const Device& device, const std::vector<vector3>& points)
{
	m_points = points;
	destroy();
	if (m_points.empty()) return m_isInitialized;

	m_lineDataBuffer.reset(new UniformBuffer());
	if (!m_lineDataBuffer->initDefaultConstant<LineRendererData>(device))
	{
		m_lineDataBuffer.reset();
		return m_isInitialized;
	}

	D3D11_BUFFER_DESC vbdesc = Geometry3D::getDefaultVertexBuffer(m_points.size() * sizeof(vector3));
	D3D11_SUBRESOURCE_DATA vbdata;
	vbdata.pSysMem = points.data();
	vbdata.SysMemPitch = 0;
	vbdata.SysMemSlicePitch = 0;
	HRESULT hr = device.device->CreateBuffer(&vbdesc, &vbdata, &m_vertexBuffer);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: couuld not initialize a 3d line. The reason: could not create a vertex buffer.\n");
		return m_isInitialized;
	}

	m_isInitialized = true;

	if (m_isInitialized) initDestroyable();
	return m_isInitialized;
}

void Line3D::renderWithStandardGpuProgram(const Device& device, const matrix44& mvp, const vector4& color)
{
	if (m_lineDataBuffer.get() == 0) return;

	auto program = StandardGpuPrograms::getLineRenderer();
	if (program->use(device))
	{
		LineRendererData data;
		data.modelViewProjection = mvp;
		data.color = color;
		m_lineDataBuffer->setData(data);
		m_lineDataBuffer->applyChanges(device);

		program->setUniform<StandardUniforms>(device, STD_UF::LINE_RENDERER_DATA, m_lineDataBuffer);
		
		render(device);
	}
}

void Line3D::render(const Device& device)
{
	if (!m_isInitialized || m_points.empty()) return;

	UINT vertexStride = sizeof(vector3);
	UINT vertexOffset = 0;
	device.context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &vertexStride, &vertexOffset);
	device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
	device.context->Draw(m_points.size(), 0);
}
	
void Line3D::destroy()
{
	m_isInitialized = false;
	if (m_vertexBuffer != 0)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

}