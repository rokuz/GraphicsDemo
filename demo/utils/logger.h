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

#ifndef __LOGGER_H__
#define __LOGGER_H__
#ifdef WIN32
    #pragma once
#endif

#include <string>

namespace utils
{

class Logger
{
public:
	enum OutputFlags
	{
		IDE_OUTPUT	= 1 << 0,
		CONSOLE		= 1 << 1,
		FILE		= 1 << 2
	};

	static void toLog(const std::string& message);
	static void toLog(const std::wstring& message);
	static void toLogWithFormat(const char* format, ...);

	static void setOutputFlags(OutputFlags flag);
	static void setOutputFlagsToDefault();

private:
	static unsigned char outputFlags;
};

}

#endif //__LOGGER_H__