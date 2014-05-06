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

#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

#include <string>
#include <map>
#include <memory>

#include "uistructs.h"
#include "fontmanager.h"
#include "overlay.h"
#include "label.h"

namespace gui
{

// factory to create user interface resources
class UIResourcesFactory
{
public:
	UIResourcesFactory(){}
	virtual ~UIResourcesFactory() {}

	virtual void cleanup() = 0;
	virtual std::weak_ptr<IFontResource> createFontResource() = 0;
};
DECLARE_PTR(UIResourcesFactory);

// user interface manager
class UIManager
{
	UIManager();
	~UIManager(){}

public:
	static UIManager& instance()
	{
		static UIManager inst;
		return inst;
	}

	bool init(size_t width, size_t height, UIResourcesFactoryPtr_T factory);
	void cleanup();

	WidgetPtr_T root() const;
	UIResourcesFactoryPtr_T factory() const { return m_factory; }

	void setScreenSize(size_t width, size_t height);

	void injectFrameTime(double elapsed);
	void injectKeyDown(InputKeys::Scan scan);
	void injectKeyUp(InputKeys::Scan scan);
	void injectChar(int character);
	void injectMouseButtonDown(InputKeys::MouseButton button);
	void injectMouseButtonUp(InputKeys::MouseButton button);
	void injectMousePosition(float x, float y);
	void injectMouseWheelChange(float delta);

	LabelPtr_T createLabel(const Coords& position, const Coords& size, 
						   Formatting hformatting = LeftAligned, 
						   Formatting vformatting = CenterAligned,
						   const std::wstring& text = L"");

	OverlayPtr_T createOverlay(const Coords& position, const Coords& size);

private:
	FontManagerPtr_T m_fontManager;
	UIResourcesFactoryPtr_T m_factory;
};


}

#endif