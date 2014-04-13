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

namespace framework
{

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Line3D::Line3D() :
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
	}

	/*glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	
	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vector3), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(vector3), 0, GL_STATIC_DRAW);
	float* vbuf = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	for (size_t i = 0; i < m_points.size(); i++)
	{
		vbuf[i * 3] =  m_points[i].x;
		vbuf[i * 3 + 1] =  m_points[i].y;
		vbuf[i * 3 + 2] =  m_points[i].z;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_isInitialized = true;*/

	if (m_isInitialized) initDestroyable();
	return m_isInitialized;
}

void Line3D::renderWithStandardGpuProgram(const Device& device, const matrix44& mvp, const vector4& color, bool closed)
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
		
		render(closed);
	}
}

void Line3D::render(bool closed)
{
	if (!m_isInitialized || m_points.empty()) return;

	//glBindVertexArray(m_vertexArray);
    //glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	//glDrawArrays(closed ? GL_LINE_LOOP : GL_LINE_STRIP, 0, m_points.size());
}
	
void Line3D::destroy()
{
	if (m_isInitialized)
	{
		//glDeleteBuffers(1, &m_vertexBuffer);
		//glDeleteVertexArrays(1, &m_vertexArray);
		m_isInitialized = false;
	}
}

}