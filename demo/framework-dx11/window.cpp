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
#include <CEGUI/InputEvent.h>

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

HWND Window::getHandle() const
{
	return m_handle;
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

std::pair<int, int> Window::size(int renderWidth, int renderHeight) const
{
	RECT rc;
	rc.top = rc.left = 0;
	rc.right = renderWidth;
	rc.bottom = renderHeight;
	AdjustWindowRect(&rc, WINDOW_STYLE, FALSE);
	return std::pair<int, int>((int)(rc.right - rc.left), (int)(rc.bottom - rc.top));
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
		case WM_SYSCHAR:
		{
			if (m_charHandler != 0) m_charHandler((int)wparam);
		}
		break;

		case WM_UNICHAR:
		{
			if (m_charHandler != 0 && wparam != UNICODE_NOCHAR) m_charHandler((int)wparam);
		}
		break;

		case WM_KEYDOWN:
		{
			if (m_keyboardHandler != 0)
			{
				int scanCode = (int)wparam;
				int keyCode = translateKey(wparam, lparam);
				m_keyboardHandler(keyCode, scanCode, true);
			}
		}
		break;

		case WM_KEYUP:
		{
			if (m_keyboardHandler != 0)
			{
				int scanCode = (int)wparam;
				int keyCode = translateKey(wparam, lparam);
				m_keyboardHandler(keyCode, scanCode, false);
			}
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void Window::setKeyboardHandler(std::function<void(int key, int scancode, bool pressed)> handler)
{
	m_keyboardHandler = handler;
}

void Window::setMouseHandler(std::function<void(double xpos, double ypos, int button, bool pressed)> handler)
{
	m_mouseHandler = handler;
}

void Window::setCharHandler(std::function<void(int codepoint)> handler)
{
	m_charHandler = handler;
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

// GLFW implementation (win32_window.c)
int Window::translateKey(WPARAM wParam, LPARAM lParam)
{
	// Check for numeric keypad keys
	// NOTE: This way we always force "NumLock = ON", which is intentional since
	//       the returned key code should correspond to a physical location.
	if ((HIWORD(lParam) & 0x100) == 0)
	{
		switch (MapVirtualKey(HIWORD(lParam) & 0xFF, 1))
		{
		case VK_INSERT:   return (int)CEGUI::Key::Scan::Numpad0;
		case VK_END:      return (int)CEGUI::Key::Scan::Numpad1;
		case VK_DOWN:     return (int)CEGUI::Key::Scan::Numpad2;
		case VK_NEXT:     return (int)CEGUI::Key::Scan::Numpad3;
		case VK_LEFT:     return (int)CEGUI::Key::Scan::Numpad4;
		case VK_CLEAR:    return (int)CEGUI::Key::Scan::Numpad5;
		case VK_RIGHT:    return (int)CEGUI::Key::Scan::Numpad6;
		case VK_HOME:     return (int)CEGUI::Key::Scan::Numpad7;
		case VK_UP:       return (int)CEGUI::Key::Scan::Numpad8;
		case VK_PRIOR:    return (int)CEGUI::Key::Scan::Numpad9;
		case VK_DIVIDE:   return (int)CEGUI::Key::Scan::Divide;
		case VK_MULTIPLY: return (int)CEGUI::Key::Scan::Multiply;
		case VK_SUBTRACT: return (int)CEGUI::Key::Scan::Subtract;
		case VK_ADD:      return (int)CEGUI::Key::Scan::Add;
		case VK_DELETE:   return (int)CEGUI::Key::Scan::Decimal;
		default:          break;
		}
	}

	// Check which key was pressed or released
	switch (wParam)
	{
	// The SHIFT keys require special handling
	case VK_SHIFT:
	{
		// Compare scan code for this key with that of VK_RSHIFT in
		// order to determine which shift key was pressed (left or
		// right)
		const DWORD scancode = MapVirtualKey(VK_RSHIFT, 0);
		if ((DWORD)((lParam & 0x01ff0000) >> 16) == scancode)
			return (int)CEGUI::Key::Scan::RightShift;

		return (int)CEGUI::Key::Scan::LeftShift;
	}

	// The CTRL keys require special handling
	case VK_CONTROL:
	{
		MSG next;
		DWORD time;

		// Is this an extended key (i.e. right key)?
		if (lParam & 0x01000000)
			return (int)CEGUI::Key::Scan::RightControl;

		// Here is a trick: "Alt Gr" sends LCTRL, then RALT. We only
		// want the RALT message, so we try to see if the next message
		// is a RALT message. In that case, this is a false LCTRL!
		time = GetMessageTime();

		if (PeekMessage(&next, NULL, 0, 0, PM_NOREMOVE))
		{
			if (next.message == WM_KEYDOWN ||
				next.message == WM_SYSKEYDOWN ||
				next.message == WM_KEYUP ||
				next.message == WM_SYSKEYUP)
			{
				if (next.wParam == VK_MENU &&
					(next.lParam & 0x01000000) &&
					next.time == time)
				{
					// Next message is a RALT down message, which
					// means that this is not a proper LCTRL message
					return (int)CEGUI::Key::Scan::Unknown;
				}
			}
		}

		return (int)CEGUI::Key::Scan::LeftControl;
	}

	// The ALT keys require special handling
	case VK_MENU:
	{
		// Is this an extended key (i.e. right key)?
		if (lParam & 0x01000000)
			return (int)CEGUI::Key::Scan::RightAlt;

		return (int)CEGUI::Key::Scan::LeftAlt;
	}

	// The ENTER keys require special handling
	case VK_RETURN:
	{
		// Is this an extended key (i.e. right key)?
		if (lParam & 0x01000000)
			return (int)CEGUI::Key::Scan::NumpadEnter;

		return (int)CEGUI::Key::Scan::Return;
	}

	// Funcion keys (non-printable keys)
	case VK_ESCAPE:        return (int)CEGUI::Key::Scan::Escape;
	case VK_TAB:           return (int)CEGUI::Key::Scan::Tab;
	case VK_BACK:          return (int)CEGUI::Key::Scan::Backspace;
	case VK_HOME:          return (int)CEGUI::Key::Scan::Home;
	case VK_END:           return (int)CEGUI::Key::Scan::End;
	case VK_PRIOR:         return (int)CEGUI::Key::Scan::PageUp;
	case VK_NEXT:          return (int)CEGUI::Key::Scan::PageDown;
	case VK_INSERT:        return (int)CEGUI::Key::Scan::Insert;
	case VK_DELETE:        return (int)CEGUI::Key::Scan::Delete;
	case VK_LEFT:          return (int)CEGUI::Key::Scan::ArrowLeft;
	case VK_UP:            return (int)CEGUI::Key::Scan::ArrowUp;
	case VK_RIGHT:         return (int)CEGUI::Key::Scan::ArrowRight;
	case VK_DOWN:          return (int)CEGUI::Key::Scan::ArrowDown;
	case VK_F1:            return (int)CEGUI::Key::Scan::F1;
	case VK_F2:            return (int)CEGUI::Key::Scan::F2;
	case VK_F3:            return (int)CEGUI::Key::Scan::F3;
	case VK_F4:            return (int)CEGUI::Key::Scan::F4;
	case VK_F5:            return (int)CEGUI::Key::Scan::F5;
	case VK_F6:            return (int)CEGUI::Key::Scan::F6;
	case VK_F7:            return (int)CEGUI::Key::Scan::F7;
	case VK_F8:            return (int)CEGUI::Key::Scan::F8;
	case VK_F9:            return (int)CEGUI::Key::Scan::F9;
	case VK_F10:           return (int)CEGUI::Key::Scan::F10;
	case VK_F11:           return (int)CEGUI::Key::Scan::F11;
	case VK_F12:           return (int)CEGUI::Key::Scan::F12;
	case VK_F13:           return (int)CEGUI::Key::Scan::F13;
	case VK_F14:           return (int)CEGUI::Key::Scan::F14;
	case VK_F15:           return (int)CEGUI::Key::Scan::F15;
	case VK_F16:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F17:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F18:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F19:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F20:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F21:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F22:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F23:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_F24:           return (int)CEGUI::Key::Scan::Unknown;
	case VK_NUMLOCK:       return (int)CEGUI::Key::Scan::NumLock;
	case VK_CAPITAL:       return (int)CEGUI::Key::Scan::Capital;
	case VK_SNAPSHOT:      return (int)CEGUI::Key::Scan::Unknown;
	case VK_SCROLL:        return (int)CEGUI::Key::Scan::ScrollLock;
	case VK_PAUSE:         return (int)CEGUI::Key::Scan::Pause;
	case VK_LWIN:          return (int)CEGUI::Key::Scan::LeftWindows;
	case VK_RWIN:          return (int)CEGUI::Key::Scan::RightWindows;
	case VK_APPS:          return (int)CEGUI::Key::Scan::AppMenu;

	// Numeric keypad
	case VK_NUMPAD0:       return (int)CEGUI::Key::Scan::Numpad0;
	case VK_NUMPAD1:       return (int)CEGUI::Key::Scan::Numpad1;
	case VK_NUMPAD2:       return (int)CEGUI::Key::Scan::Numpad2;
	case VK_NUMPAD3:       return (int)CEGUI::Key::Scan::Numpad3;
	case VK_NUMPAD4:       return (int)CEGUI::Key::Scan::Numpad4;
	case VK_NUMPAD5:       return (int)CEGUI::Key::Scan::Numpad5;
	case VK_NUMPAD6:       return (int)CEGUI::Key::Scan::Numpad6;
	case VK_NUMPAD7:       return (int)CEGUI::Key::Scan::Numpad7;
	case VK_NUMPAD8:       return (int)CEGUI::Key::Scan::Numpad8;
	case VK_NUMPAD9:       return (int)CEGUI::Key::Scan::Numpad9;
	case VK_DIVIDE:        return (int)CEGUI::Key::Scan::Divide;
	case VK_MULTIPLY:      return (int)CEGUI::Key::Scan::Multiply;
	case VK_SUBTRACT:      return (int)CEGUI::Key::Scan::Subtract;
	case VK_ADD:           return (int)CEGUI::Key::Scan::Add;
	case VK_DECIMAL:       return (int)CEGUI::Key::Scan::Decimal;

	// Printable keys are mapped according to US layout
	case VK_SPACE:         return (int)CEGUI::Key::Scan::Space;
	case 0x30:             return (int)CEGUI::Key::Scan::Zero;
	case 0x31:             return (int)CEGUI::Key::Scan::One;
	case 0x32:             return (int)CEGUI::Key::Scan::Two;
	case 0x33:             return (int)CEGUI::Key::Scan::Three;
	case 0x34:             return (int)CEGUI::Key::Scan::Four;
	case 0x35:             return (int)CEGUI::Key::Scan::Five;
	case 0x36:             return (int)CEGUI::Key::Scan::Six;
	case 0x37:             return (int)CEGUI::Key::Scan::Seven;
	case 0x38:             return (int)CEGUI::Key::Scan::Eight;
	case 0x39:             return (int)CEGUI::Key::Scan::Nine;
	case 0x41:             return (int)CEGUI::Key::Scan::A;
	case 0x42:             return (int)CEGUI::Key::Scan::B;
	case 0x43:             return (int)CEGUI::Key::Scan::C;
	case 0x44:             return (int)CEGUI::Key::Scan::D;
	case 0x45:             return (int)CEGUI::Key::Scan::E;
	case 0x46:             return (int)CEGUI::Key::Scan::F;
	case 0x47:             return (int)CEGUI::Key::Scan::G;
	case 0x48:             return (int)CEGUI::Key::Scan::H;
	case 0x49:             return (int)CEGUI::Key::Scan::I;
	case 0x4A:             return (int)CEGUI::Key::Scan::J;
	case 0x4B:             return (int)CEGUI::Key::Scan::K;
	case 0x4C:             return (int)CEGUI::Key::Scan::L;
	case 0x4D:             return (int)CEGUI::Key::Scan::M;
	case 0x4E:             return (int)CEGUI::Key::Scan::N;
	case 0x4F:             return (int)CEGUI::Key::Scan::O;
	case 0x50:             return (int)CEGUI::Key::Scan::P;
	case 0x51:             return (int)CEGUI::Key::Scan::Q;
	case 0x52:             return (int)CEGUI::Key::Scan::R;
	case 0x53:             return (int)CEGUI::Key::Scan::S;
	case 0x54:             return (int)CEGUI::Key::Scan::T;
	case 0x55:             return (int)CEGUI::Key::Scan::U;
	case 0x56:             return (int)CEGUI::Key::Scan::V;
	case 0x57:             return (int)CEGUI::Key::Scan::W;
	case 0x58:             return (int)CEGUI::Key::Scan::X;
	case 0x59:             return (int)CEGUI::Key::Scan::Y;
	case 0x5A:             return (int)CEGUI::Key::Scan::Z;
	case 0xBD:             return (int)CEGUI::Key::Scan::Minus;
	case 0xBB:             return (int)CEGUI::Key::Scan::Equals;
	case 0xDB:             return (int)CEGUI::Key::Scan::LeftBracket;
	case 0xDD:             return (int)CEGUI::Key::Scan::RightBracket;
	case 0xDC:             return (int)CEGUI::Key::Scan::Backslash;
	case 0xBA:             return (int)CEGUI::Key::Scan::Semicolon;
	case 0xDE:             return (int)CEGUI::Key::Scan::Apostrophe;
	case 0xC0:             return (int)CEGUI::Key::Scan::Grave;
	case 0xBC:             return (int)CEGUI::Key::Scan::Comma;
	case 0xBE:             return (int)CEGUI::Key::Scan::Period;
	case 0xBF:             return (int)CEGUI::Key::Scan::Slash;
	case 0xDF:             return (int)CEGUI::Key::Scan::Unknown;
	case 0xE2:             return (int)CEGUI::Key::Scan::Unknown;
	default:               break;
	}

	// No matching translation was found
	return (int)CEGUI::Key::Scan::Unknown;
}

}