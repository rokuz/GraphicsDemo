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
	bool initArray(int arraySize, int width, int height, GLint format, int samples = 0, GLint depthFormat = -1);

	int getColorBuffer(int index = 0);
	int getDepthBuffer();

	void bind(int index);
	void bindDepth();
	void bindAsImage(int targetIndex, int imageSlot, bool readFlag = true, bool writeFlag = true);

	void set();
	void clearColorAsUint(size_t index, unsigned int r = 0, unsigned int g = 0, unsigned int b = 0, unsigned int a = 0);
	void clearColorAsFloat(size_t index, const vector4& color = vector4(0, 0, 0, 0));
	void clearDepth(float depth = 1.0f);
	void copyDepthToCurrentDepthBuffer(int samplesCount = 0);
	void copyColorToBackBuffer();

private:
	virtual void destroy();

	bool initFramebuffer();
	void initColorBuffers();
	void initDepth();
	bool checkStatus();

	GLuint m_framebufferObject;
	std::vector<GLuint> m_colorBuffers;
	std::vector<GLint> m_formats;
	GLuint m_depthBuffer;
	bool m_isUsedDepth;
	int m_samples;
	int m_target;
	size_t m_width;
	size_t m_height;
	int m_depthFormat;
	size_t m_arraySize;

	bool m_isInitialized;
};


}

#endif