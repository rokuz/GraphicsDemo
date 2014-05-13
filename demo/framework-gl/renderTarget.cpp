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
#include "renderTarget.h"

namespace framework
{

RenderTarget::RenderTarget() :
	m_framebufferObject(0),
	m_depthBuffer(0),
	m_isInitialized(false),
	m_isUsedDepth(false)
{

}

RenderTarget::~RenderTarget()
{
	destroy();
}

bool RenderTarget::initWithColorBuffers(int width, int heigth, const std::vector<GLint>& formats)
{
	if (width <= 0 || heigth <= 0 || formats.empty() || formats.size() > 16)
	{
		utils::Logger::toLog("RenderTarget: Cannot initialize, incorrect parameters.\n");
		return false;
	}

	destroy();

	glGenFramebuffers(1, &m_framebufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
	initColorBuffers(width, heigth, formats);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_isInitialized = true;
	if (!checkStatus()) destroy();

	if (m_isInitialized) initDestroyable();
	return m_isInitialized;
}

bool RenderTarget::initWithColorBuffersAndDepth(int width, int heigth, const std::vector<GLint>& formats, GLint depthFormat)
{
	if (width <= 0 || heigth <= 0 || formats.empty() || formats.size() > 16)
	{
		utils::Logger::toLog("RenderTarget error: Cannot initialize, incorrect parameters.\n");
		return false;
	}

	destroy();

	glGenFramebuffers(1, &m_framebufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
	initColorBuffers(width, heigth, formats);
	initDepth(width, heigth, depthFormat);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_isInitialized = true;
	if (!checkStatus()) destroy();

	if (m_isInitialized) initDestroyable();
	return m_isInitialized;
}

void RenderTarget::initColorBuffers(int width, int heigth, const std::vector<GLint>& formats)
{
	m_colorBuffers.resize(formats.size());
	glGenTextures(formats.size(), m_colorBuffers.data());

	destroy();

	for (size_t i = 0; i < formats.size(); i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_colorBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], width, heigth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	for (size_t i = 0; i < formats.size(); i++)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_colorBuffers[i], 0);
	}
}

void RenderTarget::initDepth(int width, int heigth, GLint depthFormat)
{
	m_isUsedDepth = true;
	glGenTextures(depthFormat, &m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, depthFormat, width, heigth);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthBuffer, 0);
}

void RenderTarget::destroy()
{
	if (m_isInitialized)
	{
		glDeleteTextures(m_colorBuffers.size(), m_colorBuffers.data());
		glDeleteTextures(1, &m_depthBuffer);
		glDeleteFramebuffers(1, &m_framebufferObject);
		m_isUsedDepth = false;

		m_isInitialized = false;
	}
}

int RenderTarget::getColorBuffer(int index)
{
	if (!m_isInitialized) return -1;
	if (index < 0 || index >= (int)m_colorBuffers.size()) return -1;
	return m_colorBuffers[index];
}

int RenderTarget::getDepthBuffer()
{
	if (!m_isInitialized) return -1;
	if (!m_isUsedDepth) return -1;
	return m_depthBuffer;
}

void RenderTarget::set()
{
	if (!m_isInitialized)
	{
		utils::Logger::toLog("RenderTarget error: Render target is not initialized\n");
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
}

bool RenderTarget::checkStatus()
{
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (fboStatus)
		{
			case GL_FRAMEBUFFER_UNDEFINED: 
				utils::Logger::toLog("RenderTarget error: OpenGL framebuffer is undefined.\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: 
				utils::Logger::toLog("RenderTarget error: OpenGL framebuffer has incomplete attachments.\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: 
				utils::Logger::toLog("RenderTarget error: OpenGL framebuffer has missing attachments.\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: 
				utils::Logger::toLog("RenderTarget error: OpenGL framebuffer has incomplete draw buffer.\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: 
				utils::Logger::toLog("RenderTarget error: OpenGL framebuffer has incomplete read buffer.\n");
				return false;
			case GL_FRAMEBUFFER_UNSUPPORTED: 
				utils::Logger::toLog("RenderTarget error: Formats combination is unsupported.\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				utils::Logger::toLog("RenderTarget error: The number of samples for each attachment must be the same.\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: 
				utils::Logger::toLog("RenderTarget error: The number of layers for each attachment must be the same.\n");
				return false;
			default:
				utils::Logger::toLog("RenderTarget error: Unknown error.\n");
				return false;
		}
	}
	return true;
}

}