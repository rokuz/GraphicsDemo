/*
 * Copyright (c) 2014 Roman Kuznetsov 
 * Implementation is borrowed from GLFW
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
#include "fpscounter.h"

namespace utils
{

FpsCounter::FpsCounter() :
	m_fpsTime(0),
	m_fpsStorage(0),
	m_timeSinceLastFpsUpdate(0),
	m_averageFps(0),
	m_framesCounter(0)
{
	m_timer.init();
}

void FpsCounter::beginFrame()
{
	m_fpsTime = m_timer.getTime();
}

bool FpsCounter::endFrame()
{
	double fpsDelta = m_timer.getTime() - m_fpsTime;
	if (fpsDelta == 0) return false;

	m_timeSinceLastFpsUpdate += fpsDelta;
	m_framesCounter++;
	m_fpsStorage += (1.0 / fpsDelta);

	if (m_timeSinceLastFpsUpdate >= 1.0f)
	{
		m_averageFps = m_fpsStorage / (m_framesCounter > 0 ? (double)m_framesCounter : 1.0);
		m_timeSinceLastFpsUpdate -= 1.0f;
		m_framesCounter = 0;
		m_fpsStorage = 0;

		return true;
	}

	return false;
}

double FpsCounter::getFps() const
{
	return m_averageFps;
}

}