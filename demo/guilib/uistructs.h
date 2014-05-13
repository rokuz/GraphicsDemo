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

#ifndef __UI_STRUCTS_H__
#define __UI_STRUCTS_H__

namespace gui
{

#define DECLARE_PTR(t) typedef std::shared_ptr<t> t##Ptr_T; typedef std::weak_ptr<t> t##WeakPtr_T;

struct Coords
{
	float relativeX;
	float relativeY;
	float absoluteX;
	float absoluteY;

	Coords() : relativeX(0), relativeY(0), absoluteX(0), absoluteY(0){}
	
	Coords(float relX, float absX, float relY, float absY) : relativeX(relX), relativeY(relY), absoluteX(absX), absoluteY(absY){}
	
	static Coords Absolute(float x, float y) 
	{
		return Coords(0, x, 0, y);
	}
	
	static Coords Relative(float x, float y)
	{
		return Coords(x, 0, y, 0);
	}

	vector2 onScreen(const vector2& screenSize)
	{
		return vector2(relativeX * screenSize.x + absoluteX, relativeY * screenSize.y + absoluteY);
	}
};

enum Formatting
{
	LeftAligned = 0,
	RightAligned,
	TopAligned,
	BottomAligned,
	CenterAligned
};

enum WidgetType
{
	UnspecifiedType = 0,
	LabelType,
	OverlayType
};

}

#endif