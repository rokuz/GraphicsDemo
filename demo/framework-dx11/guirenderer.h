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
#include "uimanager.h"
#include <list>

namespace framework
{

class FontResourceD3D11 : public gui::IFontResource
{
public:
	virtual ~FontResourceD3D11(){}

	virtual bool createResource(const gui::Font& font, const std::vector<unsigned char>& buffer, size_t width, size_t height);
};

class UIResourcesFactoryD3D11 : public gui::UIResourcesFactory
{
public:
	UIResourcesFactoryD3D11(){}
	virtual ~UIResourcesFactoryD3D11() {}

	virtual void cleanup();
	virtual std::weak_ptr<gui::IFontResource> createFontResource();

private:
	std::list<std::shared_ptr<FontResourceD3D11> > m_fonts;
};

}