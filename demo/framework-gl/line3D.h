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

#ifndef __LINE_3D_H__
#define __LINE_3D_H__

#include "GL/gl3w.h"
#include "destroyable.h"
#include "matrix.h"
#include "vector.h"
#include <vector>

namespace framework
{

class Line3D : public Destroyable
{
	friend class Application;

public:
	Line3D();
	virtual  ~Line3D();

	bool initWithArray(const std::vector<vector3>& points);
	
	void renderWithStandardGpuProgram(const matrix44& mvp, const vector4& color, bool closed);
	void render(bool closed);

private:
	std::vector<vector3> m_points;
	bool m_isInitialized;
	GLuint m_vertexArray;
    GLuint m_vertexBuffer;

	virtual void destroy();
};

}

#endif // __LINE_3D_H__