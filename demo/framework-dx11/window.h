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
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <string>
#include <functional>

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
	HWND getHandle() const;
	std::pair<int, int> size(int renderWidth, int renderHeight) const;
	std::pair<double, double> getCursorPosition();
	void setCursorVisibility(bool isVisible);
	void setKeyboardHandler(std::function<void(int key, int scancode, bool pressed)> handler);
	void setCharHandler(std::function<void(int codepoint)> handler);
	void setMouseHandler(std::function<void(double xpos, double ypos, double zdelta, int button, bool pressed)> handler);

private:
	HWND m_handle;
	size_t m_width;
	size_t m_height;
	bool m_shouldClose;
	std::function<void(int, int, bool)> m_keyboardHandler;
	std::function<void(double, double, double, int, bool)> m_mouseHandler;
	std::function<void(int)> m_charHandler;

	LRESULT handleEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	int translateKey(WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK _windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};


}