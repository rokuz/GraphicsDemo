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

#include "stdafx.h"
#include "widget.h"

namespace gui
{

Widget::Widget() :
	m_visible(true)
{
}

void Widget::addChild(WidgetWeakPtr_T widget)
{
	if (widget.expired()) return;
	WidgetPtr_T widgetPtr = widget.lock();

	auto found = std::find_if(m_children.begin(), m_children.end(), [&](WidgetWeakPtr_T wPtr)
	{
		if (wPtr.expired()) return false;
		return wPtr.lock() == widgetPtr;
	});
	if (found != m_children.end()) return;

	widgetPtr->m_parent = shared_from_this();
	m_children.push_back(widgetPtr);

	if (m_renderingCache) m_renderingCache->invalidate();
}

void Widget::removeChild(WidgetWeakPtr_T widget)
{
	if (widget.expired()) return;
	WidgetPtr_T widgetPtr = widget.lock();

	auto found = std::find_if(m_children.begin(), m_children.end(), [&](WidgetWeakPtr_T wPtr)
	{
		if (wPtr.expired()) return false;
		return wPtr.lock() == widgetPtr;
	});
	if (found == m_children.end()) return;

	m_children.erase(found);
	widgetPtr->m_parent.reset();

	if (m_renderingCache) m_renderingCache->invalidate();
}

void Widget::setVisible(bool visible)
{
	m_visible = visible;
	if (m_renderingCache) m_renderingCache->invalidate();
}

void Widget::setPosition(const Coords& pos)
{
	m_position = pos;
	if (m_renderingCache) m_renderingCache->invalidate();
}

void Widget::setSize(const Coords& size)
{
	m_size = size;
	if (m_renderingCache) m_renderingCache->invalidate();
}

vector2 Widget::computeOnScreenPosition()
{
	if (UIManager::instance().root().get() == this)
	{
		return m_position.onScreen(UIManager::instance().getScreenSize());
	}
	if (m_parent.expired()) return vector2();
	vector2 parentSize = m_parent.lock()->computeOnScreenSize();

	return m_position.onScreen(parentSize);
}

vector2 Widget::computeOnScreenSize()
{
	if (UIManager::instance().root().get() == this)
	{
		return m_size.onScreen(UIManager::instance().getScreenSize());
	}
	if (m_parent.expired()) return vector2();
	vector2 parentSize = m_parent.lock()->computeOnScreenSize();

	return m_size.onScreen(parentSize);
}

void Widget::setRenderingCache(WidgetRenderingCachePtr_T cache)
{
	m_renderingCache = cache;
}

}