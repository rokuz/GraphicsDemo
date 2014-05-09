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

#ifndef __LABEL_H__
#define __LABEL_H__

#include "widget.h"
#include <string>
#include "vector.h"

namespace gui
{

class Label : public Widget
{
public:
	Label();
	virtual ~Label(){}
	virtual WidgetType getType() const { return LabelType; }

	void setText(const std::wstring& text);
	void setHorzFormatting(Formatting formatting);
	void setVertFormatting(Formatting formatting);
	void setFont(int fontId);
	void setColor(const vector4& color);

	const std::wstring& getText() const { return m_text; }
	Formatting getHorzFormatting() const { return m_horzFormatting; }
	Formatting getVertFormatting() const { return m_vertFormatting; }
	int getFont() const { return m_fontId; }
	const vector4& getColor() const { return m_color; }

private:
	std::wstring m_text;
	Formatting m_horzFormatting;
	Formatting m_vertFormatting;
	int m_fontId;
	vector4 m_color;
};

DECLARE_PTR(Label);

}

#endif