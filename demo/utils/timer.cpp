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
#include "timer.h"

namespace utils
{

#if defined _WIN32

unsigned __int64 getRawTime()
{
	unsigned __int64 time;
	QueryPerformanceCounter((LARGE_INTEGER*)&time);
	return time;
}

bool Timer::init()
{
	unsigned __int64 frequency;
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency))
	{
		m_resolution = 1.0 / (double)frequency;
	}
	else
	{
		return false;
	}

	m_base = getRawTime();
	return true;
}

double Timer::getTime()
{
	return (double)(getRawTime() - m_base) * m_resolution;
}

#elif defined __APPLE__

uint64_t getRawTime()
{
	return mach_absolute_time();
}

bool Timer::init()
{
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);

	m_resolution = (double)info.numer / (info.denom * 1.0e9);
	
	m_base = getRawTime();
	return true;
}

double Timer::getTime()
{
	return (double)(getRawTime() - m_base) * m_resolution;
}

#endif

}