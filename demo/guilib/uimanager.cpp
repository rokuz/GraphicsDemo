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
#include "utils.h"
#include "logger.h"
#include <algorithm>

namespace gui
{

UIManager::UIManager()
{
}

bool UIManager::init(size_t width, size_t height, UIResourcesFactoryPtr_T factory, UIRendererPtr_T renderer)
{
	if (!utils::Utils::exists("data/gui"))
	{
		utils::Logger::toLog("Error: could not find gui directory. Probably working directory has not been set correctly (especially if you are running from IDE).\n");
		return false;
	}

	if (width == 0 || height == 0)
	{
		utils::Logger::toLog("Error: failed to initialize ui manager, width/height could not be 0.\n");
		return false;
	}

	m_factory = factory;
	m_renderer = renderer;
	if (m_renderer && !m_renderer->init())
	{
		utils::Logger::toLog("Error: failed to initialize ui renderer.\n");
		return false;
	}
	setScreenSize(width, height);

	// font manager
	m_fontManager.reset(new FontManager());
	if (!m_fontManager->init()) return false;

	// default font
	m_defaultFont = m_fontManager->createFont("data/gui/DejaVuSans.ttf", 14);
	if (!m_defaultFont.isValid())
	{
		utils::Logger::toLog("Error: failed to initialize ui manager, could not create default font.\n");
		return false;
	}

	// root window
	m_root.reset(new Widget());
	m_root->setVisible(true);
	m_root->setPosition(Coords::Relative(0, 0));
	m_root->setSize(Coords::Relative(1, 1));

	return true;
}

void UIManager::cleanup()
{
	if (m_fontManager) 
	{
		m_fontManager->destroy();
		m_fontManager.reset();
	}

	if (m_factory)
	{
		m_factory->cleanup();
		m_factory.reset();
	}

	if (m_renderer)
	{
		m_renderer->cleanup();
		m_renderer.reset();
	}
}

WidgetPtr_T UIManager::root() const
{
	return m_root;
}

void UIManager::setScreenSize(size_t width, size_t height)
{
	m_screenSize.x = (float)width;
	m_screenSize.y = (float)height;

	if (m_root) invalidateWidget(m_root);
}

void UIManager::injectFrameTime(double elapsed)
{
	// TODO: implement
}

void UIManager::injectKeyDown(InputKeys::Scan scan)
{
	// TODO: implement
}

void UIManager::injectKeyUp(InputKeys::Scan scan)
{
	// TODO: implement
}

void UIManager::injectChar(int character)
{
	// TODO: implement
}

void UIManager::injectMouseButtonDown(InputKeys::MouseButton button)
{
	// TODO: implement
}

void UIManager::injectMouseButtonUp(InputKeys::MouseButton button)
{
	// TODO: implement
}

void UIManager::injectMousePosition(float x, float y)
{
	// TODO: implement
}

void UIManager::injectMouseWheelChange(float delta)
{
	// TODO: implement
}

void UIManager::invalidateWidget(gui::WidgetPtr_T widget)
{
	if (widget->getRenderingCache())
	{
		widget->getRenderingCache()->invalidate();
	}

	std::for_each(widget->getChildren().cbegin(), widget->getChildren().cend(), [&](gui::WidgetWeakPtr_T ptr)
	{
		if (!ptr.expired())
			invalidateWidget(std::move(ptr.lock()));
	});
}

}