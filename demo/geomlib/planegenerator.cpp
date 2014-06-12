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
#include "planegenerator.h"

namespace geom
{

void PlaneGenerator::setPlaneGenerationInfo(const PlaneGenerationInfo& info)
{
	m_info = info;
}

Data PlaneGenerator::generate()
{
	Data data;
	DataWriter writer(&data);

	if (m_info.segments[0] <= 0 || m_info.segments[1] <= 0)
	{
		writer.getLastErrorRef() = "Segments values must be >= 1";
		return data;
	}

	if (m_info.size.x <= 0 || m_info.size.y <= 0)
	{
		writer.getLastErrorRef() = "Size of a plane must not be <= 0.0";
		return data;
	}

	int sx = m_info.segments[0] + 1;
	int sy = m_info.segments[1] + 1;
	int verticesCount = sx * sy;

	writer.getMeshesRef().resize(1);
	writer.getMeshesRef()[0].offsetInIB = 0;
	writer.getMeshesRef()[0].indicesCount = m_info.segments[0] * m_info.segments[1] * 6;

	writer.getVerticesCountRef() = verticesCount;
	writer.getAdditionalUVsCountRef() = 0;
	writer.getBoundingBoxRef().vmin = vector3(-0.5f * m_info.size.x, -0.1f, -0.5f * m_info.size.y);
	writer.getBoundingBoxRef().vmax = vector3(0.5f * m_info.size.x, 0.1f, 0.5f * m_info.size.y);

	writer.getVertexBufferRef().resize(verticesCount * sizeof(Data::Vertex));
	for (int y = 0; y < sy; y++)
	{
		float pz = (float(y) / float(sy - 1) - 0.5f) * m_info.size.y;
		for (int x = 0; x < sx; x++)
		{
			int i = y * sx + x;
			float px = (float(x) / float(sx - 1) - 0.5f) * m_info.size.x;
			Data::Vertex* vertex = reinterpret_cast<Data::Vertex*>(writer.getVertexBufferRef().data() + i * sizeof(Data::Vertex));
			vertex->position = vector3(px, 0, pz);
			vertex->normal = vector3(0.0f, 1.0f, 0.0f);
			vertex->tangent = vector3(0.0f, 0.0f, 1.0f);
			vertex->binormal = vector3(1.0f, 0.0f, 0.0f);
			vertex->texCoord0 = vector2(float(x) * m_info.uvSize.x / float(sx - 1), float(y) * m_info.uvSize.y / float(sy - 1));
		}
	}

	writer.getIndexBufferRef().resize(writer.getMeshesRef()[0].indicesCount);
	unsigned int currentIndex = 0;
	for (int y = 0; y < m_info.segments[1]; y++)
	{
		unsigned int offset = (unsigned int)y * (unsigned int)sx;
		for (int x = 0; x < m_info.segments[0]; x++)
		{
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)x;
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + sx);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + sx + 1);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + sx + 1);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + 1);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)x;
		}
	}

	return data;
}

}