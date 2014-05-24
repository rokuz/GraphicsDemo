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

#ifndef __UNIFORM_BUFFER_H__
#define __UNIFORM_BUFFER_H__

namespace framework
{

class UniformBuffer : public Destroyable
{
	friend class Application;

public:
	UniformBuffer();
	virtual ~UniformBuffer();

	template<typename DataType> bool init(size_t count)
	{
		destroy();

		m_isStorage = false;
		initBuffer(sizeof(DataType) * count);
		
		if (CHECK_GL_ERROR)
		{
			destroy();
			utils::Logger::toLog("Error: could not initialize an uniform buffer.\n");
			return false;
		}

		initDestroyable();
		return true;
	}

	template<typename DataType> bool initStorage(size_t count)
	{
		destroy();

		m_isStorage = true;
		initBuffer(sizeof(DataType)* count);

		if (CHECK_GL_ERROR)
		{
			destroy();
			utils::Logger::toLog("Error: could not initialize a storage buffer.\n");
			return false;
		}

		initDestroyable();
		return true;
	}

	template<typename DataType> const DataType& getElement(int index) const
	{
		static DataType dummy;
		size_t i = index * sizeof(DataType);
		if (i < 0 || i >= m_bufferInMemory.size()) return dummy;
		return reinterpret_cast<const DataType&>(m_bufferInMemory[i]);
	}

	template<typename DataType> void setElement(int index, const DataType& value)
	{
		size_t i = index * sizeof(DataType);
		if (i < 0 || i >= m_bufferInMemory.size()) return;
		DataType& elem = reinterpret_cast<DataType&>(m_bufferInMemory[i]);
		elem = value;
		m_isChanged = true;
	}

	template<typename DataType> void size()
	{
		return m_bufferInMemory.size() / sizeof(DataType);
	}

	bool isValid() const;
	bool isStorage() const;
	void bind(int bindingIndex);

private:
	virtual void destroy();
	void initBuffer(size_t data);

	GLuint m_buffer;
	std::vector<unsigned char> m_bufferInMemory;
	bool m_isChanged;
	bool m_isStorage;
};


}

#endif