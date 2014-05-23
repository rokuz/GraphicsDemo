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
#include "atomicCounter.h"

namespace framework
{

AtomicCounter::AtomicCounter() :
	m_buffer(0)
{
}

AtomicCounter::~AtomicCounter()
{
}

bool AtomicCounter::init(unsigned int initialValue)
{
	destroy();

	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_buffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_COPY);
	GLuint data = initialValue;
	glClearBufferSubData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, 0, sizeof(GLuint), GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	if (CHECK_GL_ERROR)
	{
		destroy();
		utils::Logger::toLog("Error: could not create an atomic counter.\n");
		return false;
	}

	initDestroyable();
	return true;
}

bool AtomicCounter::isValid() const
{
	return m_buffer != 0;
}

void AtomicCounter::destroy()
{
	if (m_buffer != 0)
	{
		glDeleteBuffers(1, &m_buffer);
		m_buffer = 0;
	}
}

void AtomicCounter::clear(unsigned int value)
{
	if (!isValid()) return;
	GLuint data = value;

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_buffer);
	glClearBufferSubData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, 0, sizeof(GLuint), GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	CHECK_GL_ERROR;
}

void AtomicCounter::bind(int bindingIndex)
{
	if (!isValid()) return;
	
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindingIndex, m_buffer);
}

}