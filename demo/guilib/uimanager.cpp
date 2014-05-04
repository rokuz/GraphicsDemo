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

#include "uimanager.h"

namespace gui
{

UIManager::UIManager()
{
}

void UIManager::init(size_t width, size_t height)
{

}

void UIManager::cleanup()
{

}

WidgetPtr_T UIManager::root() const
{
	return WidgetPtr_T(new Widget());
}

void UIManager::setScreenSize(size_t width, size_t height)
{

}

void UIManager::injectFrameTime(double elapsed)
{

}

void UIManager::injectKeyDown(InputKeys::Scan scan)
{

}

void UIManager::injectKeyUp(InputKeys::Scan scan)
{

}

void UIManager::injectChar(int character)
{

}

void UIManager::injectMouseButtonDown(InputKeys::MouseButton button)
{

}

void UIManager::injectMouseButtonUp(InputKeys::MouseButton button)
{

}

void UIManager::injectMousePosition(float x, float y)
{

}

void UIManager::injectMouseWheelChange(float delta)
{

}

LabelPtr_T UIManager::createLabel(const Coords& position, const Coords& size, Formatting hformatting, Formatting vformatting, const std::wstring& text)
{
	return LabelPtr_T(new Label());
}

OverlayPtr_T UIManager::createOverlay(const Coords& position, const Coords& size)
{
	return OverlayPtr_T(new Overlay());
}

}