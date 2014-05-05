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

#include "fontmanager.h"
#include "logger.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace gui
{

const int DPI = 96;

class FreeTypeWrapper
{
public:
	FreeTypeWrapper(){}
	~FreeTypeWrapper(){}

	bool init()
	{
		if (FT_Init_FreeType(&m_library))
		{
			utils::Logger::toLog("Error: could not not initialize FreeType library.\n");
			return false;
		}
		return true;
	}

	void destroy()
	{
		FT_Done_FreeType(m_library);
	}

	bool loadFont(const std::string& name, size_t height, size_t bufferWidth, size_t bufferHeight)
	{
		FT_Face face;
		if (FT_New_Face(m_library, name.c_str(), 0, &face))
		{
			utils::Logger::toLogWithFormat("Error: could not not load font '%s'.\n", name.c_str());
			return false;
		}

		FT_Set_Char_Size(face, height << 6, height << 6, DPI, DPI);

		FT_GlyphSlot slot = face->glyph;
		for (FT_Long index = 0; index < face->num_glyphs; index++)
		{
			if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT))
			{
				FT_Done_Face(face);
				return false;
			}

			if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				FT_Done_Face(face);
				return false;
			}

			int w = (int)slot->metrics.width >> 6;
			int h = (int)slot->metrics.height >> 6;
			int ax = (int)slot->advance.x >> 6;
			int bx = (int)slot->metrics.horiBearingX >> 6;
			int by = (int)slot->metrics.horiBearingY >> 6;

			//slot->bitmap
			//pen_x += slot->advance.x >> 6;
			//pen_y += slot->advance.y >> 6; /* not useful for now */
		}

		FT_Done_Face(face);

		return true;
	}

private:
	FT_Library m_library;
};

bool FontManager::init()
{
	m_freetype.reset(new FreeTypeWrapper());
	if (!m_freetype->init()) 
	{ 
		m_freetype.reset(); 
		return false; 
	}

	m_freetype->loadFont("data/gui/DejaVuSans.ttf", 16, 2048, 2048);

	return true;
}

void FontManager::destroy()
{
	if (m_freetype) { m_freetype->destroy(); }
}

}