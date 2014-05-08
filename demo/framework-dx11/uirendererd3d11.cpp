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

#include "uirendererd3d11.h"
#include "application.h"

#include "texture.h"
#include "gpuprogram.h"

namespace framework
{

bool UIRendererD3D11::init()
{
	return true;
}

void UIRendererD3D11::cleanup()
{

}

void UIRendererD3D11::render()
{
	auto root = gui::UIManager::instance().root();
	if (root)
	{
		renderWidget(std::move(root));
	}
}

void UIRendererD3D11::renderWidget(gui::WidgetPtr_T widget)
{
	if (!widget->isVisible()) return;

	if (widget->getType() == gui::LabelType)
	{
		renderLabel(std::move(std::static_pointer_cast<gui::Label>(widget)));
	}
	else if (widget->getType() == gui::OverlayType)
	{
		// TODO
	}
	
	std::for_each(widget->getChildren().cbegin(), widget->getChildren().cend(), [&](gui::WidgetWeakPtr_T ptr)
	{
		if (!ptr.expired())
			renderWidget(std::move(ptr.lock()));
	});
}

void UIRendererD3D11::renderLabel(gui::LabelPtr_T label)
{
	int fontId = label->getFont();
	const gui::Font& font = fontId < 0 ? gui::UIManager::instance().defaultFont() :
										 gui::UIManager::instance().fontManager()->getFont(fontId);
	if (!font.isValid()) return;
	auto fontResource = std::static_pointer_cast<FontResourceD3D11>(font.getResource().lock());

	if (label->getRenderingCache().get() != nullptr)
	{
		auto cachePtr = std::static_pointer_cast<LabelRenderingCache>(label->getRenderingCache());
		if (!cachePtr->isValid())
		{
			cachePtr->position = label->computeOnScreenPosition();
			cachePtr->size = label->computeOnScreenSize();
			cachePtr->characters = font.computeCharacters(label->getText(), cachePtr->position, cachePtr->size, 
														  label->getHorzFormatting(), label->getVertFormatting());
			cachePtr->setValid();
		}
	}
	
	int a = 1;
}

}