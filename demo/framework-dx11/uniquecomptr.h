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

#ifndef __UNIQUE_COM_PTR_H__
#define __UNIQUE_COM_PTR_H__
#ifdef WIN32
    #pragma once
#endif

namespace framework
{

template<typename ComInterface>
class UniqueComPtr
{
public:
	UniqueComPtr(UniqueComPtr&& ptr)
	{
		m_ptr = std::move(ptr.m_ptr);
	}

	UniqueComPtr& operator=(UniqueComPtr&& ptr)
	{
		if (*this == &ptr) return *this;
		m_ptr = std::move(ptr.m_ptr);
		return *this;
	}

	~UniqueComPtr()
	{
		if (m_ptr != 0)
		{
			m_ptr->Release();
		}
	}

	ComInterface* get()
	{
		return m_ptr;
	}

	static UniqueComPtr Create()
	{
		return UniqueComPtr((ComInterface*)0);
	}

	static UniqueComPtr Wrap(ComInterface* ptr)
	{
		return UniqueComPtr(ptr);
	}

private:
	ComInterface* m_ptr;

	explicit UniqueComPtr(ComInterface* ptr)
	{
		m_ptr = ptr;
	}

	UniqueComPtr(const UniqueComPtr& ptr)
	{
		// copying is forbidden
	}

	UniqueComPtr& operator=(UniqueComPtr& ptr)
	{
		// copying is forbidden
		return *this;
	}
};


}

#endif