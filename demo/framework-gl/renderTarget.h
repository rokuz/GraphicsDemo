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

namespace framework
{

class RenderTarget : public Destroyable
{
	friend class Application;

public:
	RenderTarget();
	virtual ~RenderTarget();

	bool init(int width, int height, const std::vector<GLint>& formats, int samples = 0, GLint depthFormat = -1);

	int getColorBuffer(int index = 0);
	int getDepthBuffer();

	void set();
	void clearColorAsUint(size_t index, const vector4& color = vector4(0, 0, 0, 0));
	void clearColorAsFloat(size_t index, const vector4& color = vector4(0, 0, 0, 0));
	void clearDepth(float depth = 1.0f);

private:
	virtual void destroy();
	void initColorBuffers(int width, int height, const std::vector<GLint>& formats);
	void initDepth(int width, int height, GLint depthFormat);
	bool checkStatus();

	GLuint m_framebufferObject;
	std::vector<GLuint> m_colorBuffers;
	GLuint m_depthBuffer;
	bool m_isUsedDepth;
	int m_samples;
	int m_target;

	bool m_isInitialized;
};


}

#endif