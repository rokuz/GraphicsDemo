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

#include "window.h"

namespace framework
{

const char* WINDOW_CLASS_NAME = "RokuzWindow";
const DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX | WS_VISIBLE;

Window::Window() :
	m_handle(0),
	m_width(800),
	m_height(600),
	m_shouldClose(false)
{

}

Window::~Window()
{

}

bool Window::init(size_t width, size_t height, const std::string& title)
{
	if (m_width != 0) m_width = width;
	if (m_height != 0) m_height = height;

	// window class
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = _windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.hInstance = 0;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wc);

	// position & size
	RECT rc;
	rc.top = rc.left = 0;
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRect(&rc, WINDOW_STYLE, FALSE);
	long wndWidth = rc.right - rc.left;
	long wndHeight = rc.bottom - rc.top;

	int primaryMonitorWidth = GetSystemMetrics(SM_CXSCREEN);
	int primaryMonitorHeight = GetSystemMetrics(SM_CYSCREEN);
	long wndLeft = (primaryMonitorWidth - width) >> 1;
	long wndTop = (primaryMonitorHeight - height) >> 1;

	// window
	m_handle = CreateWindowEx(NULL,	wc.lpszClassName, title.c_str(), WINDOW_STYLE,
							  wndLeft, wndTop, wndWidth, wndHeight,
							  NULL, NULL, NULL, NULL);

	if (m_handle != 0)
	{
		SetWindowLongPtr(m_handle, 0, (LONG_PTR)this);
		ShowWindow(m_handle, SW_SHOWNORMAL);
		UpdateWindow(m_handle);
	}

	return m_handle != 0;
}

void Window::destroy()
{
	if (m_handle)
	{
		DestroyWindow(m_handle);
		m_handle = 0;
	}

	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));
}

void Window::pollEvents()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			m_shouldClose = true;
			return;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool Window::shouldClose() const
{
	return m_shouldClose;
}

LRESULT Window::handleEvent(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_CREATE:
			return 0;

		case WM_PAINT:
		case WM_CLOSE:
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
		{
			//EvtWindowResizePtr pEvent = EvtWindowResizePtr(new EvtWindowResize(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		} 
		break;

		case WM_LBUTTONUP:
		{
			//EvtMouseLButtonUpPtr pEvent = EvtMouseLButtonUpPtr(new EvtMouseLButtonUp(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_LBUTTONDOWN:
		{
			//EvtMouseLButtonDownPtr pEvent = EvtMouseLButtonDownPtr(new EvtMouseLButtonDown(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_MBUTTONUP:
		{
			//EvtMouseMButtonUpPtr pEvent = EvtMouseMButtonUpPtr(new EvtMouseMButtonUp(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_MBUTTONDOWN:
		{
			//EvtMouseMButtonDownPtr pEvent = EvtMouseMButtonDownPtr(new EvtMouseMButtonDown(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_RBUTTONUP:
		{
			//EvtMouseRButtonUpPtr pEvent = EvtMouseRButtonUpPtr(new EvtMouseRButtonUp(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_RBUTTONDOWN:
		{
			//EvtMouseRButtonDownPtr pEvent = EvtMouseRButtonDownPtr(new EvtMouseRButtonDown(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_MOUSEMOVE:
		{
			//EvtMouseMovePtr pEvent = EvtMouseMovePtr(new EvtMouseMove(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_MOUSEWHEEL:
		{
			//EvtMouseWheelPtr pEvent = EvtMouseWheelPtr(new EvtMouseWheel(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_CHAR:
		{
			//EvtCharPtr pEvent = EvtCharPtr(new EvtChar(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_KEYDOWN:
		{
			//EvtKeyDownPtr pEvent = EvtKeyDownPtr(new EvtKeyDown(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;

		case WM_KEYUP:
		{
			//EvtKeyUpPtr pEvent = EvtKeyUpPtr(new EvtKeyUp(hwnd, wparam, lparam));
			//EvtManager.ProcessEvent(pEvent);
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK Window::_windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LONG_PTR windowPtr = GetWindowLongPtr(hwnd, 0);

	if (windowPtr == 0)
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	else
	{
		return ((Window*)windowPtr)->handleEvent(hwnd, msg, wparam, lparam);
	}
}

}