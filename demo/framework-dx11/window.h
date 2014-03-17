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

#ifndef __WINDOW_H__
#define __WINDOW_H__
#ifdef WIN32
    #pragma once
	#define WIN32_LEAN_AND_MEAN 1
	#include <Windows.h>
#endif

#include <string>

namespace framework
{

class Window
{
public:
	Window();
	~Window();

	bool init(size_t width, size_t height, const std::string& title);
	void destroy();
	void pollEvents();
	bool shouldClose() const;

private:
	HWND m_handle;
	size_t m_width;
	size_t m_height;
	bool m_shouldClose;

	LRESULT handleEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	static LRESULT CALLBACK _windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};


}

#endif