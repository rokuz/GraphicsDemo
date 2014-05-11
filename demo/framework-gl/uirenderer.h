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

#ifndef __UI_RENDERER_H__
#define __UI_RENDERER_H__

#include "uimanager.h"
#include <list>

namespace framework
{

class GpuProgram;
class UniformBuffer;

// renderer
class UIRendererOGL : public gui::UIRenderer
{
public:
	UIRendererOGL(){}
	~UIRendererOGL(){}

	virtual bool init();
	virtual void render();
	virtual void cleanup();

private:
	void renderWidget(gui::WidgetPtr_T widget);
	void renderLabel(gui::LabelPtr_T label);

	std::shared_ptr<framework::GpuProgram> m_textRendering;
	std::shared_ptr<framework::UniformBuffer> m_textDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_charactersDataBuffer;
};

// rendering cache
class LabelRenderingCache : public gui::WidgetRenderingCache
{
public:
	vector2 position;
	vector2 size;
	std::list<gui::Font::Character> characters;
};

}

#endif