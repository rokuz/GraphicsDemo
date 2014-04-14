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

#include "geometry3D.h"

#include <list>
#include <map>
#include <algorithm>
#include "logger.h"
#include "utils.h"
#include "line3D.h"

namespace framework
{

template<typename T>
DXGI_FORMAT getComponentFormat(size_t sz)
{
	return DXGI_FORMAT_UNKNOWN;
}
template<>
DXGI_FORMAT getComponentFormat<unsigned int>(size_t sz)
{
	if (sz == 4) return DXGI_FORMAT_R32_UINT;
	else if (sz == 8) return DXGI_FORMAT_R32G32_UINT;
	else if (sz == 12) return DXGI_FORMAT_R32G32B32_UINT;
	else if (sz == 16) return DXGI_FORMAT_R32G32B32A32_UINT;
	return DXGI_FORMAT_UNKNOWN;
}
template<>
DXGI_FORMAT getComponentFormat<int>(size_t sz)
{
	if (sz == 4) return DXGI_FORMAT_R32_SINT;
	else if (sz == 8) return DXGI_FORMAT_R32G32_SINT;
	else if (sz == 12) return DXGI_FORMAT_R32G32B32_SINT;
	else if (sz == 16) return DXGI_FORMAT_R32G32B32A32_SINT;
	return DXGI_FORMAT_UNKNOWN;
}
template<>
DXGI_FORMAT getComponentFormat<float>(size_t sz)
{
	if (sz == 4) return DXGI_FORMAT_R32_FLOAT;
	else if (sz == 8) return DXGI_FORMAT_R32G32_FLOAT;
	else if (sz == 12) return DXGI_FORMAT_R32G32B32_FLOAT;
	else if (sz == 16) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	return DXGI_FORMAT_UNKNOWN;
}

Geometry3D::Geometry3D() :
    m_vertexBuffer(0),
    m_indexBuffer(0),
    m_additionalUVsCount(0),
    m_isLoaded(false),
	m_verticesCount(0),
	m_indicesCount(0),
	m_vertexSize(0)
{
    
}

Geometry3D::~Geometry3D()
{
	destroy();
}

D3D11_BUFFER_DESC Geometry3D::getDefaultVertexBuffer(unsigned int size)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = size;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.CPUAccessFlags = 0;
	return desc;
}
D3D11_BUFFER_DESC Geometry3D::getDefaultIndexBuffer(unsigned int size)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = size;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.CPUAccessFlags = 0;
	return desc;
}

void Geometry3D::destroy()
{
	m_isLoaded = false;
    m_inputLayoutInfo.clear();
	
	if (m_vertexBuffer != 0)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	if (m_indexBuffer != 0)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	m_boundingBoxLine.reset();
}
bool Geometry3D::init(const Device& device, const std::string& fileName)
{
	destroy();

	geom::Data data = geom::Geometry::instance().load(fileName);
	if (!data.isCorrect())
	{
		m_isLoaded = false;
		return m_isLoaded;
	}

	m_boundingBox = data.getBoundingBox();
	m_additionalUVsCount = data.getAdditionalUVsCount();
	m_meshes = data.getMeshes();
	m_verticesCount = data.getVerticesCount();
	m_indicesCount = data.getIndexBuffer().size();
	m_vertexSize = data.getVertexSize();

	HRESULT hr = S_OK;

	// input layout
	m_inputLayoutInfo.resize(data.getVertexComponentsCount());
	for (size_t component = 0; component < data.getVertexComponentsCount(); component++)
	{
		size_t offset = data.getVertexComponentOffset(component);
		size_t componentSize = data.getVertexComponentSize(component);

		m_inputLayoutInfo[component].SemanticName = data.getSemanticName(component);
		m_inputLayoutInfo[component].SemanticIndex = data.getSemanticIndex(component);
		m_inputLayoutInfo[component].Format = getComponentFormat<float>(componentSize);
		m_inputLayoutInfo[component].InputSlot = 0;
		m_inputLayoutInfo[component].AlignedByteOffset = offset;
		m_inputLayoutInfo[component].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		m_inputLayoutInfo[component].InstanceDataStepRate = 0;
	}

	// vertex buffer
	D3D11_BUFFER_DESC vbdesc = getDefaultVertexBuffer(data.getVertexBuffer().size());
	D3D11_SUBRESOURCE_DATA vbdata;
	vbdata.pSysMem = data.getVertexBuffer().data();
	vbdata.SysMemPitch = 0;
	vbdata.SysMemSlicePitch = 0;
	hr = device.device->CreateBuffer(&vbdesc, &vbdata, &m_vertexBuffer);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create a vertex buffer.\n");
		return m_isLoaded;
	}
	
	// index buffer
	D3D11_BUFFER_DESC ibdesc = getDefaultIndexBuffer(m_indicesCount * sizeof(unsigned int));
	D3D11_SUBRESOURCE_DATA ibdata;
	ibdata.pSysMem = data.getIndexBuffer().data();
	ibdata.SysMemPitch = 0;
	ibdata.SysMemSlicePitch = 0;
	hr = device.device->CreateBuffer(&ibdesc, &ibdata, &m_indexBuffer);
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create an index buffer.\n");
		return m_isLoaded;
	}

	m_isLoaded = true;

	if (m_isLoaded) initDestroyable();
	return m_isLoaded;
}

size_t Geometry3D::getMeshesCount() const
{
    return m_meshes.size();
}

void Geometry3D::renderMesh(const Device& device, size_t index)
{
    if (index >= m_meshes.size()) return;

	UINT vertexStride = m_vertexSize;
	UINT vertexOffset = 0;
	device.context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &vertexStride, &vertexOffset);
	device.context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device.context->DrawIndexed(m_meshes[index].indicesCount, m_meshes[index].offsetInIB, 0);
}

void Geometry3D::renderAllMeshes(const Device& device)
{
	UINT vertexStride = m_vertexSize;
	UINT vertexOffset = 0;
	device.context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &vertexStride, &vertexOffset);
	device.context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		device.context->DrawIndexed(m_meshes[i].indicesCount, m_meshes[i].offsetInIB, 0);
	}
}

void Geometry3D::renderBoundingBox(const Device& device, const matrix44& mvp)
{
	static bool failed = false;
	if (m_boundingBoxLine.get() == 0 && !failed)
	{
		int indices[] = { 0, 1, 2, 3, 0, 6, 5, 1, 5, 4, 2, 4, 7, 3, 7, 6 };
		std::vector<vector3> points;
		points.resize(sizeof(indices) / sizeof(indices[0]));
		for (int i = 0; i < sizeof(indices) / sizeof(indices[0]); i++)
		{
			points[i] = m_boundingBox.corner_point(indices[i]);
		}
		
		m_boundingBoxLine.reset(new Line3D());
		if (!m_boundingBoxLine->initWithArray(device, points))
		{
			utils::Logger::toLog("Error: could not render a bounding box.\n");
			failed = true;
			m_boundingBoxLine.reset();
		}
	}

	if (m_boundingBoxLine.get() != 0)
	{
		m_boundingBoxLine->renderWithStandardGpuProgram(device, mvp, vector4(1, 1, 0, 1));
	}
}

const std::vector<D3D11_INPUT_ELEMENT_DESC>& Geometry3D::getInputLayoutInfo() const
{
	return m_inputLayoutInfo;
}

}