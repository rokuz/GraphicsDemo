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

#ifndef __GEOMETRY_DATA_H__
#define __GEOMETRY_DATA_H__

namespace geom
{

class Data
{
	friend class GeometryLoader;
	friend class GeometryGenerator;

public:
	struct Vertex
	{                       // indices:
		vector3 position;   // 0
		vector3 normal;     // 1
		vector2 texCoord0;  // 2
		vector3 tangent;    // 3
		vector3 binormal;   // 4
	};

	struct Material
	{
		std::string diffuseMapFilename;
		std::string normalMapFilename;
		std::string specularMapFilename;
	};

	struct Mesh
	{
		size_t offsetInIB;
		size_t indicesCount;
		Material material;
		Mesh() : offsetInIB(0), indicesCount(0) {}
	};

	typedef std::vector<Mesh> Meshes;

	Data();
	Data(const Data& data);
	Data(Data&& data);

	~Data(){}

	bool isCorrect() const;
	const std::string& getLastError() const;
	const Meshes& getMeshes() const;
	size_t getAdditionalUVsCount() const;
	size_t getVerticesCount() const;
	const bbox3& getBoundingBox() const;
	const std::vector<unsigned char>& getVertexBuffer() const;
	const std::vector<unsigned int>& getIndexBuffer() const;

	size_t getVertexComponentsCount() const;
	size_t getVertexComponentSize(size_t index) const;
	size_t getVertexComponentOffset(size_t index) const;
	size_t getVertexSize() const;
	const char* getSemanticName(size_t index) const;
	size_t getSemanticIndex(size_t index) const;

	Data& operator=(const Data& data);
	Data& operator=(Data&& data);

private:
	Meshes m_meshes;
	size_t m_additionalUVsCount;
	size_t m_verticesCount;
	bbox3 m_boundingBox;
	std::vector<unsigned char> m_vertexBuffer;
	std::vector<unsigned int> m_indexBuffer;
	std::string m_lastError;
};


}

#endif