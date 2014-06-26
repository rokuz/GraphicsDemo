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
#include "geometry3D.h"

#include "planegenerator.h"

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
	m_vertexSize(0),
	m_id(-1)
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
	m_inputLayoutCache.clear();
	
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

bool Geometry3D::init(const std::string& fileName, bool calculateAdjacency)
{
	destroy();

	geom::Data data = geom::Geometry::instance().load(fileName);
	if (!data.isCorrect())
	{
		m_isLoaded = false;
		utils::Logger::toLogWithFormat("Error: could not load geometry from '%s'.\n", fileName.c_str());
		return m_isLoaded;
	}
	m_filename = fileName;

	return init(data, calculateAdjacency);
}

bool Geometry3D::initAsPlane(const geom::PlaneGenerationInfo& info, bool calculateAdjacency)
{
	destroy();
	
	geom::PlaneGenerator generator;
	generator.setPlaneGenerationInfo(info);

	geom::Data data = generator.generate();
	if (!data.isCorrect())
	{
		m_isLoaded = false;
		utils::Logger::toLogWithFormat("Error: could not create a plane.\n");
		return m_isLoaded;
	}

	return init(data, calculateAdjacency);
}

bool Geometry3D::initAsTerrain(const geom::TerrainGenerationInfo& info, bool calculateAdjacency)
{
	destroy();

	geom::TerrainGenerator generator;
	generator.setTerrainGenerationInfo(info);

	geom::Data data = generator.generate();
	if (!data.isCorrect())
	{
		m_isLoaded = false;
		utils::Logger::toLogWithFormat("Error: could not create a terrain.\n");
		return m_isLoaded;
	}

	return init(data, calculateAdjacency);
}

bool Geometry3D::init(const geom::Data& data, bool calculateAdjacency)
{
	const Device& device = Application::instance()->getDevice();

	if (calculateAdjacency)
	{
		m_adjacency = data.calculateAdjacency();
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
	initDestroyable();
	m_id = generateId();

	return m_isLoaded;
}

void Geometry3D::bindToGpuProgram(std::shared_ptr<GpuProgram> program)
{
	auto i = getInputLayoutBindingIndex(program->getId());
	if (i < 0)
	{
		int index = program->bindInputLayoutInfo(m_inputLayoutInfo);
		m_inputLayoutCache.push_back(std::make_pair(program->getId(), index));
	}
}

int Geometry3D::getInputLayoutBindingIndex(int programId) const
{
	auto it = std::find_if(m_inputLayoutCache.begin(),
						   m_inputLayoutCache.end(),
						   [&](const InputLayoutPair_T& p)->bool
	{
		return p.first == programId;
	});

	if (it != m_inputLayoutCache.end()) return it->second;
	return -1;
}

size_t Geometry3D::getMeshesCount() const
{
    return m_meshes.size();
}

void Geometry3D::applyInputLayout()
{
	auto gpuProgram = Application::instance()->getUsingGpuProgram();
	if (!gpuProgram.expired())
	{
		auto gpuProgramPtr = gpuProgram.lock();
		int inputLayoutIndex = getInputLayoutBindingIndex(gpuProgramPtr->getId());
		gpuProgramPtr->applyInputLayout(inputLayoutIndex);
	}
}

void Geometry3D::renderMesh(size_t index, size_t instancesCount)
{
    if (index >= m_meshes.size()) return;

	const Device& device = Application::instance()->getDevice();

	applyInputLayout();
	UINT vertexStride = m_vertexSize;
	UINT vertexOffset = 0;
	device.context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &vertexStride, &vertexOffset);
	device.context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if (instancesCount <= 1)
	{
		device.context->DrawIndexed(m_meshes[index].indicesCount, m_meshes[index].offsetInIB, 0);
	}
	else
	{
		device.context->DrawIndexedInstanced(m_meshes[index].indicesCount, instancesCount, m_meshes[index].offsetInIB, 0, 0);
	}
}

void Geometry3D::renderAllMeshes(size_t instancesCount)
{
	const Device& device = Application::instance()->getDevice();

	applyInputLayout();
	UINT vertexStride = m_vertexSize;
	UINT vertexOffset = 0;
	device.context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &vertexStride, &vertexOffset);
	device.context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		if (instancesCount <= 1)
		{
			device.context->DrawIndexed(m_meshes[i].indicesCount, m_meshes[i].offsetInIB, 0);
		}
		else
		{
			device.context->DrawIndexedInstanced(m_meshes[i].indicesCount, instancesCount, m_meshes[i].offsetInIB, 0, 0);
		}
	}
}

void Geometry3D::renderBoundingBox(const matrix44& mvp)
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
		if (!m_boundingBoxLine->initWithArray(points))
		{
			utils::Logger::toLog("Error: could not render a bounding box.\n");
			failed = true;
			m_boundingBoxLine.reset();
		}
	}

	if (m_boundingBoxLine.get() != 0)
	{
		m_boundingBoxLine->renderWithStandardGpuProgram(mvp, vector4(1, 1, 0, 1));
	}
}

int Geometry3D::generateId()
{
	static int counter = 0;
	return counter++;
}

}