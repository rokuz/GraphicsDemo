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
#include "gpuprogram.h"

namespace framework
{

static GLenum COLOR_ATTACHMENTS[] = 
{
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7,
	GL_COLOR_ATTACHMENT8,
	GL_COLOR_ATTACHMENT9,
	GL_COLOR_ATTACHMENT10,
	GL_COLOR_ATTACHMENT11,
	GL_COLOR_ATTACHMENT12,
	GL_COLOR_ATTACHMENT13,
	GL_COLOR_ATTACHMENT14,
	GL_COLOR_ATTACHMENT15
};

RenderTarget::RenderTarget() :
	m_framebufferObject(0),
	m_depthBuffer(0),
	m_isInitialized(false),
	m_isUsedDepth(false),
	m_width(0),
	m_height(0),
	m_samples(0),
	m_target(-1)
{

}

RenderTarget::~RenderTarget()
{
	destroy();
}

bool RenderTarget::init(int width, int height, const std::vector<GLint>& formats, int samples, GLint depthFormat)
{
	if (width <= 0 || height <= 0 || formats.empty() || formats.size() > 16)
	{
		utils::Logger::toLog("Error: could not initialize a render target, incorrect parameters.\n");
		return false;
	}

	destroy();

	m_width = width;
	m_height = height;
	m_samples = samples;
	m_target = GL_TEXTURE_2D;
	if (m_samples > 0) m_target = GL_TEXTURE_2D_MULTISAMPLE;

	glGenFramebuffers(1, &m_framebufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
	initColorBuffers(width, height, formats);
	if (depthFormat >= 0)
	{
		initDepth(width, height, depthFormat);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!checkStatus() || CHECK_GL_ERROR) 
	{
		destroy();
		return false;
	}

	m_isInitialized = true;
	initDestroyable();
	return m_isInitialized;
}

void RenderTarget::initColorBuffers(int width, int height, const std::vector<GLint>& formats)
{
	m_colorBuffers.resize(formats.size());
	glGenTextures(formats.size(), m_colorBuffers.data());

	for (size_t i = 0; i < formats.size(); i++)
	{
		glBindTexture(m_target, m_colorBuffers[i]);
		if (m_samples == 0)
		{
			glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexStorage2D(m_target, 1, formats[i], width, height);
		}
		else
		{
			glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage2DMultisample(m_target, m_samples, formats[i], width, height, GL_TRUE);
		}
	}

	glBindTexture(m_target, 0);

	for (size_t i = 0; i < formats.size(); i++)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, COLOR_ATTACHMENTS[i], m_colorBuffers[i], 0);
	}
}

void RenderTarget::initDepth(int width, int height, GLint depthFormat)
{
	m_isUsedDepth = true;
	glGenTextures(1, &m_depthBuffer);
	glBindTexture(m_target, m_depthBuffer);
	if (m_samples == 0)
	{
		glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
		glTexStorage2D(m_target, 1, depthFormat, width, height);
	}
	else
	{
		glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2DMultisample(m_target, m_samples, depthFormat, width, height, GL_TRUE);
	}
	glBindTexture(m_target, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthBuffer, 0);
}

void RenderTarget::destroy()
{
	if (!m_colorBuffers.empty())
	{
		glDeleteTextures(m_colorBuffers.size(), m_colorBuffers.data());
	}
	m_colorBuffers.clear();

	if (m_depthBuffer != 0)
	{
		glDeleteTextures(1, &m_depthBuffer);
	}

	if (m_framebufferObject != 0)
	{
		glDeleteFramebuffers(1, &m_framebufferObject);
	}
	
	m_isUsedDepth = false;
	m_isInitialized = false;
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
	if (!m_isInitialized) return;

	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
	glDrawBuffers(m_colorBuffers.size(), COLOR_ATTACHMENTS);
	CHECK_GL_ERROR;
}

void RenderTarget::clearColorAsUint(size_t index, const vector4& color)
{
	if (!m_isInitialized) return;

	const GLuint c[] = { (GLuint)color.x, (GLuint)color.y, (GLuint)color.z, (GLuint)color.w };
	glClearBufferuiv(GL_COLOR, index, c);
}

void RenderTarget::clearColorAsFloat(size_t index, const vector4& color)
{
	if (!m_isInitialized) return;

	const GLfloat c[] = { color.x, color.y, color.z, color.w };
	glClearBufferfv(GL_COLOR, index, c);
}

void RenderTarget::clearDepth(float depth)
{
	if (!m_isInitialized) return;
	if (!m_isUsedDepth) return;

	glClearBufferfv(GL_DEPTH, 0, &depth);
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

void RenderTarget::bind( int index )
{
	if (!m_isInitialized) return;
	if (index < 0 || index >= (int)m_colorBuffers.size()) return;

	glBindTexture(m_target, m_colorBuffers[index]);
}

void RenderTarget::bindDepth()
{
	if (!m_isInitialized || !m_isUsedDepth) return;

	glBindTexture(m_target, m_depthBuffer);
}

void RenderTarget::copyDepthToCurrentDepthBuffer(int samplesCount)
{
	auto prog = StandardGpuPrograms::getDepthBufferCopying(samplesCount > 0);
	if (prog->use())
	{
		auto self = std::static_pointer_cast<RenderTarget>(shared_from_this());
		prog->setDepth<StandardUniforms>(STD_UF::DEPTH_MAP, self);
		if (samplesCount > 0)
		{
			prog->setInt<StandardUniforms>(STD_UF::SAMPLES_COUNT, samplesCount);
		}

		DepthState depthEnabled(true);
		depthEnabled.setWriteEnable(true);
		depthEnabled.apply();

		ColorOutputState colorOutputDisable(false);
		colorOutputDisable.apply();
		
		glDrawArrays(GL_POINTS, 0, 1);
		
		depthEnabled.cancel();
		colorOutputDisable.cancel();
	}
}

}