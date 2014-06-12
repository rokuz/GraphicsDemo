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

namespace framework
{

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Geometry3D::Geometry3D() :
    m_vertexArray(0),
    m_vertexBuffer(0),
    m_indexBuffer(0),
    m_additionalUVsCount(0),
    m_isLoaded(false),
	m_verticesCount(0),
	m_indicesCount(0),
	m_id(-1)
{
    
}

Geometry3D::~Geometry3D()
{
	destroy();
}

void Geometry3D::destroy()
{
	if (m_vertexBuffer != 0)
	{
		glDeleteBuffers(1, &m_vertexBuffer);
		m_vertexBuffer = 0;
	}

	if (m_indexBuffer != 0)
	{
		glDeleteBuffers(1, &m_indexBuffer);
		m_indexBuffer = 0;
	}

	if (m_vertexArray != 0)
	{
        glDeleteVertexArrays(1, &m_vertexArray);
		m_vertexArray = 0;
    }

	m_boundingBoxLine.reset();

	m_isLoaded = false;
}

bool Geometry3D::init(const std::string& fileName)
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

	return init(data);
}

bool Geometry3D::initAsPlane(const geom::PlaneGenerationInfo& info)
{
	destroy();

	geom::PlaneGenerator generator;
	generator.setPlaneGenerationInfo(info);

	geom::Data data = generator.generate();
	if (!data.isCorrect())
	{
		m_isLoaded = false;
		utils::Logger::toLog("Error: could not create a plane.\n");
		return m_isLoaded;
	}

	return init(data);
}

bool Geometry3D::initAsTerrain(const geom::TerrainGenerationInfo& info)
{
	destroy();

	geom::TerrainGenerator generator;
	generator.setTerrainGenerationInfo(info);

	geom::Data data = generator.generate();
	if (!data.isCorrect())
	{
		m_isLoaded = false;
		utils::Logger::toLog("Error: could not create a terrain.\n");
		return m_isLoaded;
	}

	return init(data);
}

bool Geometry3D::init(const geom::Data& data)
{
	m_boundingBox = data.getBoundingBox();
	m_additionalUVsCount = data.getAdditionalUVsCount();
	m_meshes = data.getMeshes();
	m_verticesCount = data.getVerticesCount();
	m_indicesCount = data.getIndexBuffer().size();

	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);
	for (size_t component = 0; component < data.getVertexComponentsCount(); component++)
	{
		size_t offset = data.getVertexComponentOffset(component);
		size_t componentSize = data.getVertexComponentSize(component);

		// vertex declaration
		glVertexAttribPointer((GLuint)component,
							  (GLuint)(componentSize / sizeof(float)),
							  GL_FLOAT,
							  GL_FALSE,
							  (GLuint)data.getVertexSize(),
							  BUFFER_OFFSET(offset));
		glEnableVertexAttribArray((GLuint)component);
	}
	glBindVertexArray(0);

	// vertex buffer
	glBufferData(GL_ARRAY_BUFFER, data.getVertexBuffer().size(), data.getVertexBuffer().data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// index buffer
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indicesCount * sizeof(unsigned int), data.getIndexBuffer().data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (CHECK_GL_ERROR)
	{
		destroy();
		return false;
	}

	m_isLoaded = true;
	initDestroyable();
	m_id = generateId();

	return m_isLoaded;
}

size_t Geometry3D::getMeshesCount() const
{
    return m_meshes.size();
}

void Geometry3D::renderMesh(size_t index, size_t instancesCount)
{
    if (index >= m_meshes.size() || instancesCount == 0) return;
    
    glBindVertexArray(m_vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	if (instancesCount == 1)
	{
		glDrawElements(GL_TRIANGLES, (int)m_meshes[index].indicesCount, GL_UNSIGNED_INT, (const GLvoid *)(m_meshes[index].offsetInIB * sizeof(unsigned int)));
	}
	else
	{
		glDrawElementsInstanced(GL_TRIANGLES, (int)m_meshes[index].indicesCount, GL_UNSIGNED_INT, (const GLvoid *)(m_meshes[index].offsetInIB * sizeof(unsigned int)), instancesCount);
	}	
}

void Geometry3D::renderAllMeshes(size_t instancesCount)
{
	if (instancesCount == 0) return;

	glBindVertexArray(m_vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		if (instancesCount == 1)
		{
			glDrawElements(GL_TRIANGLES, (int)m_meshes[i].indicesCount, GL_UNSIGNED_INT, (const GLvoid *)(m_meshes[i].offsetInIB * sizeof(unsigned int)));
		}
		else
		{
			glDrawElementsInstanced(GL_TRIANGLES, (int)m_meshes[i].indicesCount, GL_UNSIGNED_INT, (const GLvoid *)(m_meshes[i].offsetInIB * sizeof(unsigned int)), instancesCount);
		}
	}
}

void Geometry3D::renderBoundingBox(const matrix44& mvp)
{
	if (!m_boundingBoxLine)
	{
		int indices[] = { 0, 1, 2, 3, 0, 6, 5, 1, 5, 4, 2, 4, 7, 3, 7, 6 };
		std::vector<vector3> points;
		points.resize(sizeof(indices) / sizeof(indices[0]));
		for (int i = 0; i < sizeof(indices) / sizeof(indices[0]); i++)
		{
			points[i] = m_boundingBox.corner_point(indices[i]);
		}
		
		m_boundingBoxLine.reset(new framework::Line3D());
		if (!m_boundingBoxLine->initWithArray(points))
		{
			m_boundingBoxLine.reset();
		}
	}

	if (m_boundingBoxLine)
	{
		m_boundingBoxLine->renderWithStandardGpuProgram(mvp, vector4(1, 1, 0, 1), false);
	}
}

int Geometry3D::generateId()
{
	static int counter = 0;
	return counter++;
}

}