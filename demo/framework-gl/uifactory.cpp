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

#include "uifactory.h"
#include "application.h"
#include "uirenderer.h"

namespace framework
{

gui::LabelPtr_T UIFactory::createLabel(const gui::Coords& position, const gui::Coords& size, gui::Formatting hformatting, gui::Formatting vformatting, const std::wstring& text)
{
	gui::LabelPtr_T label(new gui::Label());
	label->setPosition(position);
	label->setSize(size);
	label->setHorzFormatting(hformatting);
	label->setVertFormatting(vformatting);
	label->setText(text);
	label->setRenderingCache(gui::WidgetRenderingCachePtr_T(new LabelRenderingCache()));
	return label;
}

gui::OverlayPtr_T UIFactory::createOverlay(const gui::Coords& position, const gui::Coords& size)
{
	gui::OverlayPtr_T overlay(new gui::Overlay());
	overlay->setPosition(position);
	overlay->setSize(size);

	return overlay;
}

bool FontResourceOGL::createResource(const gui::Font& font, const std::vector<unsigned char>& buffer, size_t width, size_t height)
{
	m_texture.reset(new Texture());
	if (!m_texture->initWithData(GL_R8, buffer, width, height))
	{
		return false;
	}

	//saveTextureToTga("font.tga", m_texture);

	return true;
}

std::weak_ptr<gui::IFontResource> UIResourcesFactoryOGL::createFontResource()
{
	std::shared_ptr<FontResourceOGL> font(new FontResourceOGL());
	m_fonts.push_back(font);
	return font;
}

void UIResourcesFactoryOGL::cleanup()
{
	m_fonts.clear();
}

}