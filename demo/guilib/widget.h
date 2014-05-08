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

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <memory>
#include <list>
#include "uistructs.h"
#include "vector.h"

namespace gui
{

class WidgetRenderingCache
{
public:
	WidgetRenderingCache() : m_isValid(false){}
	virtual ~WidgetRenderingCache(){}
	
	void invalidate() { m_isValid = false; }
	bool isValid() const { return m_isValid; }
	void setValid() { m_isValid = true; }

protected:
	bool m_isValid;
};
DECLARE_PTR(WidgetRenderingCache);

class Widget;
DECLARE_PTR(Widget);

class Widget : public std::enable_shared_from_this<Widget>
{
public:
	Widget();
	virtual ~Widget(){}
	
	virtual WidgetType getType() const { return UnspecifiedType; }

	void addChild(WidgetWeakPtr_T widget);
	void removeChild(WidgetWeakPtr_T widget);

	void setVisible(bool visible);
	void setPosition(const Coords& pos);
	void setSize(const Coords& size);
	
	const Coords& getPosition() const { return m_position; }
	const Coords& getSize() const { return m_size; }
	bool isVisible() const { return m_visible; }
	const std::list<WidgetWeakPtr_T>& getChildren() const { return m_children; }

	vector2 computeOnScreenPosition();
	vector2 computeOnScreenSize();

	void setRenderingCache(WidgetRenderingCachePtr_T cache);
	WidgetRenderingCachePtr_T getRenderingCache() const { return m_renderingCache; }

protected:
	Coords m_position;
	Coords m_size;
	bool m_visible;
	WidgetWeakPtr_T m_parent;
	std::list<WidgetWeakPtr_T> m_children;
	WidgetRenderingCachePtr_T m_renderingCache;
};


}

#endif