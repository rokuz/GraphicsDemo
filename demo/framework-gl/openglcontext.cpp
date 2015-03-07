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

#include "stdafx.h"
#include "openglcontext.h"

#pragma comment(lib, "opengl32.lib")

namespace framework
{

bool checkForOpenGLError(const char* file, const char* function, int line)
{
	GLenum err(glGetError());

	bool result = false;
	while (err != GL_NO_ERROR) 
	{
		result = true;
		std::string error;

		switch (err) {
		case GL_INVALID_OPERATION:      
			error = "INVALID_OPERATION";      
			break;
		case GL_INVALID_ENUM:           
			error = "INVALID_ENUM";           
			break;
		case GL_INVALID_VALUE:          
			error = "INVALID_VALUE";          
			break;
		case GL_OUT_OF_MEMORY:         
			error = "OUT_OF_MEMORY";          
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  
			error = "INVALID_FRAMEBUFFER_OPERATION";  
			break;
		}

		utils::Logger::toLogWithFormat("OpenGL error: %s (%s > %s, line: %d).\n", error.c_str(), file, function, line);
		err = glGetError();
	}

	return result;
}

bool OpenGLContext::init( size_t width, size_t height, const std::string& title, bool fullscreen, int samples, bool vsync, bool enableStencil )
{
	// check extensions
	if (!getWGLExtensions())
	{
		return false;
	}
	if (!hasWGLExtension("WGL_ARB_create_context") || !hasWGLExtension("WGL_ARB_pixel_format"))
	{
		return false;
	}
	if (samples > 0 && !hasWGLExtension("WGL_ARB_multisample"))
	{
		return false;
	}

	// create window
	if (!m_window.init(width, height, title, fullscreen))
	{
		return false;
	}

	// initialize rendering context
	if (!initRenderingContext(samples, vsync, enableStencil))
	{
		// desired contest's unsupported
		destroy();
		return false;
	}

	if (gl3wInit() < 0)
	{
		// unsupported OGL 3+
		destroy();
		return false;
	}

	return true;
}

void OpenGLContext::destroy()
{
	destroy(m_window.getHandle());
	m_window.destroy();
}

void OpenGLContext::destroy( HWND handle )
{
	wglMakeCurrent(NULL, NULL);
	if (m_renderingContext)
	{
		wglDeleteContext(m_renderingContext);
		m_renderingContext = 0;
	}
	if (m_deviceContext)
	{
		::ReleaseDC(handle, m_deviceContext);
		m_deviceContext = 0;
	}
}

void OpenGLContext::present()
{
	TRACE_FUNCTION;
	if (!m_deviceContext) return;

	::SwapBuffers(m_deviceContext);
}

Window& OpenGLContext::getWindow()
{
	return m_window;
}

bool OpenGLContext::initRenderingContext( int samples, bool vsync, bool enableStencil )
{
	m_deviceContext = ::GetDC(m_window.getHandle());
	PIXELFORMATDESCRIPTOR desc;
	::ZeroMemory(&desc, sizeof(desc));

	int pixelFormat = getPixelFormat(samples, enableStencil);
	if (pixelFormat < 0)
	{
		return false;
	}

	if (::SetPixelFormat(m_deviceContext, pixelFormat, &desc) == 0) 
	{
		return false;
	}

	for (int minorVersion = 3; minorVersion >= 0; minorVersion--)
	{
		int attributeList[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, minorVersion,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		m_renderingContext = wglCreateContextAttribsARB(m_deviceContext, 0, attributeList);
		if (m_renderingContext) break;
	}
	if (!m_renderingContext)
	{
		return false;
	}

	if (!wglMakeCurrent(m_deviceContext, m_renderingContext))
	{
		return false;
	}

	bool vsyncResult = false;
	if (vsync)
	{
		vsyncResult = (wglSwapIntervalEXT(1) != 0);
	}
	else
	{
		vsyncResult = (wglSwapIntervalEXT(0) != 0);
	}

	if (!vsyncResult)
	{
		return false;
	}

	return true;
}

bool OpenGLContext::getWGLExtensions()
{
	framework::Window tmpWindow;
	tmpWindow.setWindowClass("tmp");
	if (!tmpWindow.init(800, 600, "tmp", false, false))
	{
		return false;
	}
	m_deviceContext = ::GetDC(tmpWindow.getHandle());

	PIXELFORMATDESCRIPTOR desc;
	::ZeroMemory(&desc, sizeof(desc));
	if (::SetPixelFormat(m_deviceContext, 1, &desc) == 0) 
	{
		return false;
	}

	m_renderingContext = wglCreateContext(m_deviceContext);
	if (!m_renderingContext) 
	{
		return false;
	}
	if (!wglMakeCurrent(m_deviceContext, m_renderingContext))
	{
		return false;
	}

	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");

	std::string wglExtensions = wglGetExtensionsStringARB(m_deviceContext);
	int index = 0;
	int previousIndex = -1;
	while (true)
	{
		index = (int)wglExtensions.find_first_of(' ', previousIndex + 1);
		if (index == std::string::npos) break;

		std::string ext = wglExtensions.substr(previousIndex + 1, index - previousIndex - 1);
		if (ext.empty()) break;

		m_wglExtensions.push_back(ext);
		previousIndex = index;
	}

	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if(!wglChoosePixelFormatARB)
	{
		return false;
	}

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if(!wglCreateContextAttribsARB)
	{
		return false;
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if(!wglSwapIntervalEXT)
	{
		return false;
	}

	destroy(tmpWindow.getHandle());

	tmpWindow.destroy();

	return true;
}

bool OpenGLContext::hasWGLExtension( const std::string& extension ) const
{
	auto it = std::find(m_wglExtensions.cbegin(), m_wglExtensions.cend(), extension);
	return it != m_wglExtensions.cend();
}

int OpenGLContext::getPixelFormat( int samples, bool enableStencil )
{
	if (!wglChoosePixelFormatARB) return -1;

	int pixelFormat = 0;
	unsigned int pixCount = 0;
	int depths[] = { 32, 24, 16 };
	for (int i = 0; i < 3; i++)
	{
		int pixAttribs[] = 
		{
			WGL_SUPPORT_OPENGL_ARB, 1,
			WGL_DRAW_TO_WINDOW_ARB, 1,
			WGL_RED_BITS_ARB, 8, WGL_GREEN_BITS_ARB, 8, WGL_BLUE_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, depths[i],
			WGL_STENCIL_BITS_ARB, (enableStencil ? 8 : 0),
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_SAMPLES_ARB, samples,
			WGL_DOUBLE_BUFFER_ARB, 1,
			WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
			0
		};

		wglChoosePixelFormatARB(m_deviceContext, &pixAttribs[0], NULL, 1, &pixelFormat, (unsigned int*)&pixCount);
		if (pixCount > 0 && pixelFormat >= 0)
		{
			return pixelFormat;
		}
	}

	return -1;
}

bool OpenGLContext::isExtensionSupported( const char* extname ) const
{
	GLint numExtensions;
	GLint i;

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	for (i = 0; i < numExtensions; i++)
	{
		const GLubyte * e = glGetStringi(GL_EXTENSIONS, i);
		if (!strcmp((const char*)e, extname))
		{
			return true;
		}
	}

	return false;
}

std::vector<int> OpenGLContext::getMultisamplingLevels()
{
	std::vector<int> multisamplingLevels;

	GLint samplesCount = 0;
	glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1, &samplesCount);
	GLint* samples = new GLint[samplesCount];
	glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, samplesCount, samples);
	multisamplingLevels.reserve(samplesCount);
	for (int i = 0; i < samplesCount; i++) 
	{
		if (samples[i] > 0) multisamplingLevels.push_back(samples[i]);
	}
	multisamplingLevels.push_back(0);
	delete [] samples;
	std::sort(multisamplingLevels.begin(), multisamplingLevels.end());

	return multisamplingLevels;
}

void OpenGLContext::makeCurrent()
{
	if (m_deviceContext != 0 && m_renderingContext != 0)
	{
		wglMakeCurrent(m_deviceContext, m_renderingContext);
	}
}

}