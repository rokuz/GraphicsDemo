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

#ifndef __RENDER_TARGET_H__
#define __RENDER_TARGET_H__

#include "GL/gl3w.h"
#include "destroyable.h"
#include <vector>

namespace framework
{

class RenderTarget : public Destroyable
{
	friend class Application;

public:
	RenderTarget();
	virtual ~RenderTarget();

	bool initWithColorBuffers(int width, int heigth, const std::vector<GLint>& formats);
	bool initWithColorBuffersAndDepth(int width, int heigth, const std::vector<GLint>& formats, GLint depthFormat);

	int getColorBuffer(int index = 0);
	int getDepthBuffer();

	void set();

private:
	virtual void destroy();
	void initColorBuffers(int width, int heigth, const std::vector<GLint>& formats);
	void initDepth(int width, int heigth, GLint depthFormat);
	bool checkStatus();

	GLuint m_framebufferObject;
	std::vector<GLuint> m_colorBuffers;
	GLuint m_depthBuffer;
	bool m_isUsedDepth;

	bool m_isInitialized;
};


}

#endif