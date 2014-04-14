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

#pragma once
#include "structs.h"
#include "geometry.h"
#include "destroyable.h"
#include "vector.h"
#include "bbox.h"
#include <string>
#include <vector>

namespace framework
{

class Line3D;

class Geometry3D : public Destroyable
{
	friend class Application;

    geom::Data::Meshes m_meshes;
    size_t m_additionalUVsCount;
	size_t m_verticesCount;
	size_t m_indicesCount;
	size_t m_vertexSize;
	bbox3 m_boundingBox;

	std::vector<D3D11_INPUT_ELEMENT_DESC> m_inputLayoutInfo;
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	bool m_isLoaded;
	std::shared_ptr<Line3D> m_boundingBoxLine;
    
	virtual void destroy();
    
public:
    Geometry3D();
    virtual ~Geometry3D();

	static D3D11_BUFFER_DESC getDefaultVertexBuffer(unsigned int size);
	static D3D11_BUFFER_DESC getDefaultIndexBuffer(unsigned int size);
	
	bool init(const Device& device, const std::string& fileName);
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& getInputLayoutInfo() const;
    
    size_t getMeshesCount() const;
    void renderMesh(const Device& device, size_t index);
	void renderAllMeshes(const Device& device);
	void renderBoundingBox(const Device& device, const matrix44& mvp);
};

}