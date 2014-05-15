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

namespace framework
{

class OpenGLContext
{
public:
	bool init(size_t width, size_t height, const std::string& title, bool fullscreen, int samples, bool vsync, bool enableStencil);
	void destroy();
	void present();
	Window& getWindow();

	bool isExtensionSupported(const char* extname) const;
	std::vector<int> getMultisamplingLevels();

private:
	Window m_window;
	HDC m_deviceContext;
	HGLRC m_renderingContext;

	std::list<std::string> m_wglExtensions;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

	void destroy(HWND handle);
	bool initRenderingContext(int samples, bool vsync, bool enableStencil);
	bool getWGLExtensions();
	bool hasWGLExtension(const std::string& extension) const;
	int getPixelFormat(int samples, bool enableStencil);
};

bool checkForOpenGLError(const char* file, const char* function, int line);
#define CHECK_GL_ERROR framework::checkForOpenGLError(__FILE__, __FUNCTION__, __LINE__)

}