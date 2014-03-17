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

#ifndef __RENDERING_SETTINGS_H__
#define __RENDERING_SETTINGS_H__
#ifdef WIN32
    #pragma once
#endif

#include <vector>

namespace CEGUI
{
	class DefaultWindow;
	class Combobox;
	class EventArgs;
}

namespace framework
{

class IRenderingSettingsHandler
{
public:
	virtual ~IRenderingSettingsHandler(){}
	virtual void onChangeMultisamplingLevel(int level){}
};

class RenderingSettings
{
public:
	RenderingSettings();
	~RenderingSettings();

	void init(CEGUI::DefaultWindow* window, const std::vector<int>& multisamplingLevels, int multisamplingLevel);
	void setHandler(IRenderingSettingsHandler* handler);

	bool handleMultisamplingChanging(const CEGUI::EventArgs& e);

private:
	IRenderingSettingsHandler* m_handler;
	std::vector<int> m_multisamplingLevels;
	CEGUI::Combobox* m_multisamplingCombo;
};

}

#endif