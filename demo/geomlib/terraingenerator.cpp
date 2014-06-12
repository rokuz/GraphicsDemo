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
#include "terraingenerator.h"

namespace geom
{

void TerrainGenerator::setTerrainGenerationInfo(const TerrainGenerationInfo& info)
{
	m_info = info;
}

Data TerrainGenerator::generate()
{
	Data data;
	DataWriter writer(&data);

	m_terrainWidth = (m_info.heightmapWidth % 2 == 0) ? m_info.heightmapWidth : m_info.heightmapWidth - 1;
	m_terrainHeight = (m_info.heightmapHeight % 2 == 0) ? m_info.heightmapHeight : m_info.heightmapHeight - 1;
	if (m_terrainWidth < 2 || m_terrainHeight < 2)
	{
		writer.getLastErrorRef() = "Heightmap size must be more than 2x2";
		return data;
	}

	int verticesCount = m_terrainWidth * m_terrainHeight;

	writer.getMeshesRef().resize(1);
	writer.getMeshesRef()[0].offsetInIB = 0;
	writer.getMeshesRef()[0].indicesCount = (m_terrainWidth - 1)* (m_terrainHeight - 1) * 6;

	writer.getVerticesCountRef() = verticesCount;
	writer.getAdditionalUVsCountRef() = 0;
	writer.getBoundingBoxRef().vmin = vector3(-0.5f * m_info.size.x, -0.5f * m_info.size.z, -0.5f * m_info.size.y);
	writer.getBoundingBoxRef().vmax = vector3(0.5f * m_info.size.x, 0.5f * m_info.size.z, 0.5f * m_info.size.y);

	writer.getVertexBufferRef().resize(verticesCount * sizeof(Data::Vertex));
	
	for (int y = 0; y < m_terrainHeight; y++)
	{
		for (int x = 0; x < m_terrainWidth; x++)
		{
			int i = y * m_terrainWidth + x;
			Data::Vertex* vertex = reinterpret_cast<Data::Vertex*>(writer.getVertexBufferRef().data() + i * sizeof(Data::Vertex));
			vertex->position = getPoint(x, y);
			calculateTangentSpace(x, y, vertex->normal, vertex->tangent, vertex->binormal);
			vertex->texCoord0 = vector2(float(x) * m_info.uvSize.x / float(m_terrainWidth - 1), float(y) * m_info.uvSize.y / float(m_terrainHeight - 1));
		}
	}

	writer.getIndexBufferRef().resize(writer.getMeshesRef()[0].indicesCount);
	unsigned int currentIndex = 0;
	for (int y = 0; y < m_terrainHeight - 1; y++)
	{
		unsigned int offset = (unsigned int)y * (unsigned int)m_terrainWidth;
		for (int x = 0; x < m_terrainWidth - 1; x++)
		{
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)x;
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + m_terrainWidth);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + m_terrainWidth + 1);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + m_terrainWidth + 1);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)(x + 1);
			writer.getIndexBufferRef()[currentIndex++] = offset + (unsigned int)x;
		}
	}

	return data;
}

vector3 TerrainGenerator::getPoint(int x, int y)
{
	vector3 pnt;
	pnt.x = m_info.size.x * (float(x) / float(m_terrainWidth - 1) - 0.5f);
	pnt.z = m_info.size.y * (float(y) / float(m_terrainHeight - 1) - 0.5f);

	int index = y * m_info.heightmapWidth + x;
	int h = m_info.heightmap[index];
	pnt.y = m_info.size.z * (float(h) / 255.0f - 0.5f);

	return pnt;
}

void TerrainGenerator::calculateTangentSpace(int x, int y, vector3& normal, vector3& tangent, vector3& binormal)
{
	vector3 center = getPoint(x, y);

	if (x + 1 < m_terrainWidth)
	{
		if (y + 1 < m_terrainHeight)
		{
			vector3 a = getPoint(x, y + 1) - center;
			a.norm();
			tangent += a;
			vector3 b = getPoint(x + 1, y) - center;
			b.norm();
			binormal += b;
			normal += (a * b);
		}
		if (y - 1 >= 0)
		{
			vector3 a = getPoint(x + 1, y) - center;
			a.norm();
			tangent += a;
			vector3 b = getPoint(x, y - 1) - center;
			b.norm();
			binormal += b;
			normal += (a * b);
		}
	}
	if (x - 1 >= 0)
	{
		if (y + 1 < m_terrainHeight)
		{
			vector3 a = getPoint(x - 1, y) - center;
			a.norm();
			tangent += a;
			vector3 b = getPoint(x, y + 1) - center;
			b.norm();
			binormal += b;
			normal += (a * b);
		}
		if (y - 1 >= 0)
		{
			vector3 a = getPoint(x, y - 1) - center;
			a.norm();
			tangent += a;
			vector3 b = getPoint(x - 1, y) - center;
			b.norm();
			binormal += b;
			normal += (a * b);
		}
	}

	normal.norm();
	tangent.norm();
	binormal.norm();
}

}