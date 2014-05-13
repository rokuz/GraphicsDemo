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

#ifndef __GEOMETRY_LOADER_H__
#define __GEOMETRY_LOADER_H__

namespace geom
{

class GeometryLoader
{
public:
	GeometryLoader(){}
	virtual ~GeometryLoader(){}
	
	virtual Data load(const std::string& filename);

protected:
	class DataWriter
	{
	public:
		DataWriter(Data* data) : m_data(data) {}

		std::string& getLastErrorRef() { return m_data->m_lastError; }
		Data::Meshes& getMeshesRef() { return m_data->m_meshes; }
		size_t& getAdditionalUVsCountRef() { return m_data->m_additionalUVsCount; }
		size_t& getVerticesCountRef() { return m_data->m_verticesCount; }
		bbox3& getBoundingBoxRef() { return m_data->m_boundingBox; }
		std::vector<unsigned char>& getVertexBufferRef() { return m_data->m_vertexBuffer; }
		std::vector<unsigned int>& getIndexBufferRef() { return m_data->m_indexBuffer; }

	private:
		Data* m_data;
	};
};

}

#endif