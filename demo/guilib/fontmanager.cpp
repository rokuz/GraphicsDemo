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
#include "fontmanager.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace gui
{

const int DPI = 96;
const int BASIC_CHARS_COUNT = 1280;
const float LINE_DIST_COEF = 1.15f;

namespace 
{

int findNearestPower2(int v)
{
	int i = 2;
	while (i < v) i <<= 1;
	return i;
}

bool isOutside(const vector4& box, const vector2& rectPos, const vector2& rectSize)
{
	float xmin = rectPos.x;
	float xmax = rectPos.x + rectSize.x;
	if ((box.x < xmin && box.z < xmin) || (box.x > xmax && box.z > xmax)) return true;
	float ymin = rectPos.y;
	float ymax = rectPos.y + rectSize.y;
	return (box.y < ymin && box.w < ymin) || (box.y > ymax && box.w > ymax);
}

}

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

	bool loadFont(Font& font, const std::string& name, size_t height)
	{
		// create font resource
		if (!UIManager::instance().factory())
		{
			utils::Logger::toLog("Error: could not find an instance of UIResourcesFactory.\n");
			return false;
		}

		FT_Face face;
		if (FT_New_Face(m_library, name.c_str(), 0, &face))
		{
			utils::Logger::toLogWithFormat("Error: could not not load font '%s'.\n", name.c_str());
			return false;
		}

		FT_Set_Char_Size(face, height << 6, height << 6, DPI, DPI);

		// build name
		std::stringstream str;
		str << face->family_name << " " << height;
		font.m_name = str.str();

		// init maximum buffer
		const int MAX_BUFFER_SIZE = 2048;
		std::unique_ptr<unsigned char[]> fontBuffer(new unsigned char[MAX_BUFFER_SIZE * MAX_BUFFER_SIZE]);
		memset(fontBuffer.get(), 0, MAX_BUFFER_SIZE * MAX_BUFFER_SIZE);

		int offsetX = 0;
		int offsetY = 0;
		int maxHeight = 0;
		int maxWidth = 0;
		FT_GlyphSlot slot = face->glyph;
		
		font.m_charToGlyph.resize(BASIC_CHARS_COUNT);
		font.m_charToGlyph.reserve(BASIC_CHARS_COUNT);
		std::map<unsigned int, unsigned int> glyphs;
		unsigned int charToGlyphIndex = 0;
		for (short index = 0; index < BASIC_CHARS_COUNT; index++)
		{
			// create association between characters and glyphs
			FT_UInt glyphIndex = FT_Get_Char_Index(face, index);
			if (glyphs.find(glyphIndex) == glyphs.end()) 
			{
				glyphs[glyphIndex] = charToGlyphIndex;
			}
			else 
			{
				font.m_charToGlyph[index] = glyphs[glyphIndex];
				continue;
			}
			
			// load a glyph
			if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
			{
				utils::Logger::toLogWithFormat("Error: could not create a font '%s'. A glyph loading failed.\n", font.getName().c_str());
				FT_Done_Face(face);
				return false;
			}

			// render a glyph
			if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				utils::Logger::toLogWithFormat("Error: could not create a font '%s'. A glyph rendering failed.\n", font.getName().c_str());
				FT_Done_Face(face);
				return false;
			}

			// read glyph's parameters
			int w = (int)slot->metrics.width >> 6;
			int h = (int)slot->metrics.height >> 6;
			int ax = (int)slot->advance.x >> 6;
			int bx = (int)slot->metrics.horiBearingX >> 6;
			int by = (int)slot->metrics.horiBearingY >> 6;
			if (w > 255 || h > 255 || ax > 255 || bx > 127 || by > 127 || bx < -128 || by < -128)
			{
				utils::Logger::toLogWithFormat("Error: could not create a font '%s'. Maximum glyph's metrics were exceeded.\n", font.getName().c_str());
				FT_Done_Face(face);
				return false;
			}

			if (offsetX + w >= MAX_BUFFER_SIZE)
			{
				offsetX = 0;
				offsetY += (maxHeight + 1);
				maxHeight = h;
			}
			if (offsetY + h >= MAX_BUFFER_SIZE)
			{
				utils::Logger::toLogWithFormat("Error: could not create a font '%s'. Maximum buffer size was exceeded.\n", font.getName().c_str());
				FT_Done_Face(face);
				return false;
			}

			if (h > maxHeight) maxHeight = h;

			// copy a glyph to buffer
			for (int j = 0; j < h; j++) 
			{
				for (int i = 0; i < w; i++)
				{
					fontBuffer[(i + offsetX) + MAX_BUFFER_SIZE * (j + offsetY)] = slot->bitmap.buffer[i + w * j];
				}
			}

			// save metrics
			Font::Glyph fontGlyph;
			fontGlyph.x = (unsigned short)offsetX;
			fontGlyph.y = (unsigned short)offsetY;
			fontGlyph.width = (unsigned char)w;
			fontGlyph.height = (unsigned char)h;
			fontGlyph.advance = (unsigned char)ax;
			fontGlyph.bearingX = (char)bx;
			fontGlyph.bearingY = (char)by;
			font.m_glyphs.push_back(fontGlyph);

			float underline = float(h - by);
			if (underline > font.m_maxUnderlineDistance) font.m_maxUnderlineDistance = underline;

			float fby = (float)abs(by);
			if (fby > font.m_linesDistance) font.m_linesDistance = fby;

			font.m_charToGlyph[index] = charToGlyphIndex;
			charToGlyphIndex++;

			offsetX += (w + 1);
			if (offsetX > maxWidth + 1) maxWidth = offsetX - 1;
		}

		font.m_linesDistance *= LINE_DIST_COEF;

		FT_Done_Face(face);

		// determine actual buffer size
		size_t textureWidth = findNearestPower2(maxWidth);
		size_t textureHeight = findNearestPower2(offsetY + maxHeight);

		std::vector<unsigned char> outputBuffer;
		outputBuffer.resize(textureWidth * textureHeight);
		memset(outputBuffer.data(), 0, textureWidth * textureHeight);
		for (size_t j = 0; j < textureHeight; j++)
		{
			memcpy((void*)(outputBuffer.data() + j * textureWidth), fontBuffer.get() + j * MAX_BUFFER_SIZE, textureWidth);
		}
		fontBuffer.reset();

		// create a resource
		font.m_resource = UIManager::instance().factory()->createFontResource();
		if (font.m_resource.expired() ||
			!font.m_resource.lock()->createResource(font, outputBuffer, textureWidth, textureHeight))
		{
			utils::Logger::toLogWithFormat("Error: could not create a resource for the font '%s'.\n", font.getName().c_str());
			return false;
		}

		return true;
	}

private:
	FT_Library m_library;
};

Font::Font() : m_id(-1), m_linesDistance(0), m_maxUnderlineDistance(0)
{
}

bool Font::isValid() const
{
	return m_id >= 0 && !m_resource.expired();
}

float Font::computeStringWidth(const std::wstring& str, size_t offset, size_t len) const
{
	if (str.empty()) return 0;
	if (len == 0) len = str.length();
	if (offset >= offset + len - 1) return 0;

	float w = 0;
	for (size_t i = offset; i < offset + len; i++)
	{
		wchar_t character = (str[i] < BASIC_CHARS_COUNT ? str[i] : L'?');
		unsigned int glyph = m_charToGlyph[character];
		const Glyph& glyphData = m_glyphs[glyph];
		w += (float)glyphData.advance;
	}

	return w;
}

float Font::computeStringUnderline(const std::wstring& str, size_t offset, size_t len) const
{
	if (str.empty()) return 0;
	if (len == 0) len = str.length();
	if (offset >= offset + len - 1) return 0;

	float underline = 0;
	for (size_t i = offset; i < offset + len; i++)
	{
		wchar_t character = (str[i] < BASIC_CHARS_COUNT ? str[i] : L'?');
		unsigned int glyph = m_charToGlyph[character];
		const Glyph& glyphData = m_glyphs[glyph];
		float ul = (float)glyphData.height - (float)glyphData.bearingY;
		if (ul > underline) underline = ul;
	}

	return underline;
}

std::list<Font::Character> Font::computeCharacters(const std::wstring& str, const vector2& rectPos, const vector2& rectSize, gui::Formatting horz, gui::Formatting vert) const
{
	std::list<Character> result;
	auto tokens = utils::Utils::tokenize<std::wstring>(str, '\n');
	if (tokens.empty()) return result;

	// calculate y-offset
	float offsetY = rectPos.y;
	if (vert == gui::BottomAligned)
	{
		float underline = computeStringUnderline(str);
		offsetY = rectPos.y + rectSize.y - m_linesDistance * tokens.size() - underline;
	}
	else if (vert == gui::CenterAligned)
	{
		offsetY = rectPos.y + 0.5f * (rectSize.y - m_linesDistance * tokens.size());
	}

	// for each string of text
	std::for_each(tokens.begin(), tokens.end(), [&](const std::pair<size_t, size_t>& pos)
	{
		// calculate x-offset
		float offsetX = rectPos.x;
		if (horz == gui::RightAligned)
		{
			float w = computeStringWidth(str, pos.first, pos.second - pos.first + 1);
			offsetX = rectPos.x + rectSize.x - w;
		}
		else if (horz == gui::CenterAligned)
		{
			float w = computeStringWidth(str, pos.first, pos.second - pos.first + 1);
			offsetX = rectPos.x + 0.5f * (rectSize.x - w);
		}

		for (size_t index = pos.first; index <= pos.second; index++)
		{
			wchar_t character = (str[index] < BASIC_CHARS_COUNT ? str[index] : L'?');
			unsigned int glyph = m_charToGlyph[character];
			const Glyph& glyphData = m_glyphs[glyph];

			if (glyphData.width != 0 && glyphData.height != 0)
			{
				vector4 box;
				box.x = offsetX + (float)glyphData.bearingX;
				box.y = offsetY + m_linesDistance - (float)glyphData.bearingY;
				box.z = box.x + (float)glyphData.width;
				box.w = box.y + (float)glyphData.height;

				if (!isOutside(box, rectPos, rectSize))
				{
					Character c;
					c.box = box;
					c.texturePos = vector2((float)glyphData.x, (float)glyphData.y);
					result.push_back(std::move(c));
				}
			}

			offsetX += (float)glyphData.advance;
		}

		offsetY += m_linesDistance;
	});

	return result;
}

bool FontManager::init()
{
	m_freetype.reset(new FreeTypeWrapper());
	if (!m_freetype->init()) 
	{ 
		m_freetype.reset(); 
		return false; 
	}

	m_fonts.reserve(10);

	return true;
}

void FontManager::destroy()
{
	if (m_freetype) { m_freetype->destroy(); }
}

Font FontManager::createFont(const std::string& fontPath, size_t height)
{
	static int idGenerator = 0;
	Font font;
	if (!m_freetype->loadFont(font, fontPath, height)) 
		return std::move(Font());

	font.m_id = idGenerator;
	idGenerator++;

	m_fonts.push_back(font);

	return std::move(font);
}

const Font& FontManager::getFont(int id) const
{
	static Font dummy;
	if (id >= (int)m_fonts.size() || id < 0) return dummy;
	return m_fonts[id];
}


}