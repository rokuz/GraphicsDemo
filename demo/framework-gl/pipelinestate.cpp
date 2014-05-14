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
	m_wasEnabled = (glIsEnabled(m_state) != 0);
	if (m_wasEnabled != m_isEnabled)
	{
		if (m_isEnabled) glEnable(m_state); else glDisable(m_state);
	}
}

void PipelineState::cancel()
{
	if (m_wasEnabled != m_isEnabled)
	{
		if (m_wasEnabled) glEnable(m_state); else glDisable(m_state);
	}
}

}