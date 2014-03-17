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

#include "uniformBuffer.h"

namespace framework
{

UniformBuffer::UniformBuffer() :
	m_buffer(0),
	m_isInitialized(false),
	m_isChanged(false)
{
}

UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::initBuffer(size_t sz)
{
	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);

	m_bufferInMemory.resize(sz);
	memset(m_bufferInMemory.data(), 0, m_bufferInMemory.size());
}

void UniformBuffer::destroy()
{
	if (m_isInitialized)
	{
		glDeleteBuffers(1, &m_buffer);
		m_isInitialized = false;
		m_isChanged = false;
	}
}

void UniformBuffer::bind(int bindingIndex)
{
	if (!m_isInitialized) return;
	if (m_isChanged)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
		glBufferData(GL_UNIFORM_BUFFER, m_bufferInMemory.size(), m_bufferInMemory.data(), GL_DYNAMIC_DRAW);
		m_isChanged = false;
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, m_buffer);
}

}