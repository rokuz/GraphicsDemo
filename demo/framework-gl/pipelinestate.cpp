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
#include "pipelinestate.h"

namespace framework
{

PipelineState::PipelineState(GLenum state, bool enabled) :
	m_state(state),
	m_isEnabled(enabled),
	m_wasEnabled(false)
{
}

PipelineState::~PipelineState()
{
}

void PipelineState::apply()
{
	if (m_state < 0) return;

	m_wasEnabled = (glIsEnabled(m_state) != 0);
	if (m_wasEnabled != m_isEnabled)
	{
		if (m_isEnabled) glEnable(m_state); else glDisable(m_state);
	}
}

void PipelineState::cancel()
{
	if (m_state < 0) return;

	if (m_wasEnabled != m_isEnabled)
	{
		if (m_wasEnabled) glEnable(m_state); else glDisable(m_state);
	}
}

BlendState::BlendState(bool enabled) :
	m_pipelineState(GL_BLEND, enabled),
	m_oldBlendSrc(-1),
	m_oldBlendDst(-1),
	m_blendSrc(-1),
	m_blendDst(-1)
{
}

void BlendState::setBlending(GLint src, GLint dest)
{
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_oldBlendSrc);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &m_oldBlendDst);

	m_blendSrc = src;
	m_blendDst = dest;
}

void BlendState::apply()
{
	m_pipelineState.apply();

	if (m_blendSrc != m_oldBlendSrc || m_blendDst != m_oldBlendDst)
	{
		glBlendFunc(m_blendSrc, m_blendDst);
	}
}

void BlendState::cancel()
{
	m_pipelineState.cancel();

	if (m_blendSrc != m_oldBlendSrc || m_blendDst != m_oldBlendDst)
	{
		glBlendFunc(m_oldBlendSrc, m_oldBlendDst);
	}
}

DepthState::DepthState(bool enabled) :
	m_pipelineState(GL_DEPTH_TEST, enabled),
	m_oldDepthWriteMask(-1),
	m_depthWriteMask(-1)
{
}

void DepthState::setWriteEnable(bool enable)
{
	GLboolean enabled;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &enabled);
	m_oldDepthWriteMask = enabled;
	m_depthWriteMask = enable ? GL_TRUE : GL_FALSE;
}

void DepthState::apply()
{
	m_pipelineState.apply();

	if (m_oldDepthWriteMask != m_depthWriteMask)
	{
		glDepthMask((GLboolean)m_depthWriteMask);
	}
}

void DepthState::cancel()
{
	m_pipelineState.cancel();

	if (m_oldDepthWriteMask != m_depthWriteMask)
	{
		glDepthMask((GLboolean)m_oldDepthWriteMask);
	}
}

ColorOutputState::ColorOutputState(bool enabled) :
	m_isEnabled(enabled)
{
}

void ColorOutputState::apply()
{
	glGetBooleanv(GL_COLOR_WRITEMASK, m_oldColorMask);
	GLboolean enabled = m_isEnabled ? GL_TRUE : GL_FALSE;
	if (m_oldColorMask[0] != enabled || m_oldColorMask[1] != enabled ||
		m_oldColorMask[2] != enabled || m_oldColorMask[3] != enabled)
	{
		glColorMask(enabled, enabled, enabled, enabled);
	}
}

void ColorOutputState::cancel()
{
	GLboolean enabled = m_isEnabled ? GL_TRUE : GL_FALSE;
	if (m_oldColorMask[0] != enabled || m_oldColorMask[1] != enabled ||
		m_oldColorMask[2] != enabled || m_oldColorMask[3] != enabled)
	{
		glColorMask(m_oldColorMask[0], m_oldColorMask[1], m_oldColorMask[2], m_oldColorMask[3]);
	}
}

}