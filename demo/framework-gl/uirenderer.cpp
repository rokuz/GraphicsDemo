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
#include "uirenderer.h"

namespace framework
{

const std::string GUI_SHADERS_PATH = "data/gui/shaders/gl/win32/";
const size_t MAX_CHARACTERS_PER_CALL = 256;

DECLARE_UNIFORMS_BEGIN(UIUniforms)
	HALF_SCREEN_SIZE,
	TEXTURE_SIZE,
	CHARACTERS_DATA,
	TEXT_COLOR,
	AREA,
	CHARACTERS_MAP
DECLARE_UNIFORMS_END()
#define UIUF UniformBase<UIUniforms>::Uniform

#pragma pack (push, 1)
struct CharacterData
{
	vector4 rectangle;
	vector2 uv;
	unsigned int : 32;
	unsigned int : 32;
};
#pragma pack (pop)

bool UIRendererOGL::init()
{
	// text rendering shader
	m_textRendering.reset(new framework::GpuProgram());
	m_textRendering->addShader(GUI_SHADERS_PATH + "text.vsh.glsl");
	m_textRendering->addShader(GUI_SHADERS_PATH + "text.gsh.glsl");
	m_textRendering->addShader(GUI_SHADERS_PATH + "text.fsh.glsl");
	if (!m_textRendering->init())
	{
		utils::Logger::toLog("Error: could not initialize text rendering shader.\n");
		return false;
	}
	m_textRendering->bindUniformBuffer<UIUniforms>(UIUF::CHARACTERS_DATA, "charactersData");
	m_textRendering->bindUniform<UIUniforms>(UIUF::HALF_SCREEN_SIZE, "halfScreenSize");
	m_textRendering->bindUniform<UIUniforms>(UIUF::TEXTURE_SIZE, "textureSize");
	m_textRendering->bindUniform<UIUniforms>(UIUF::TEXT_COLOR, "textColor");
	m_textRendering->bindUniform<UIUniforms>(UIUF::AREA, "area");
	m_textRendering->bindUniform<UIUniforms>(UIUF::CHARACTERS_MAP, "charactersMap");

	m_charactersDataBuffer.reset(new framework::UniformBuffer());
	if (!m_charactersDataBuffer->init<framework::CharacterData>(MAX_CHARACTERS_PER_CALL))
	{
		utils::Logger::toLog("Error: could not initialize characters data buffer.\n");
		return false;
	}

	return true;
}

void UIRendererOGL::cleanup()
{
	m_charactersDataBuffer.reset();
	m_textRendering.reset();
}

void UIRendererOGL::render()
{
	auto root = gui::UIManager::instance().root();
	if (root)
	{
		renderWidget(std::move(root));
	}
}

void UIRendererOGL::renderWidget(gui::WidgetPtr_T widget)
{
	if (!widget->isVisible()) return;

	if (widget->getType() == gui::LabelType)
	{
		renderLabel(std::move(std::static_pointer_cast<gui::Label>(widget)));
	}
	else if (widget->getType() == gui::OverlayType)
	{
		// TODO: implement
	}
	
	std::for_each(widget->getChildren().cbegin(), widget->getChildren().cend(), [&](gui::WidgetWeakPtr_T ptr)
	{
		if (!ptr.expired())
			renderWidget(std::move(ptr.lock()));
	});
}

void UIRendererOGL::renderLabel(gui::LabelPtr_T label)
{
	int fontId = label->getFont();
	const gui::Font& font = fontId < 0 ? gui::UIManager::instance().defaultFont() :
										 gui::UIManager::instance().fontManager()->getFont(fontId);
	if (!font.isValid()) return;
	auto fontResource = std::static_pointer_cast<FontResourceOGL>(font.getResource().lock());

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
		if (cachePtr->characters.empty()) return;		

		// rendering
		if (m_textRendering->use())
		{
			//Application::instance()->disableDepthTest()->apply();
			//Application::instance()->defaultAlphaBlending()->apply();

			GLboolean depthTestEnable = glIsEnabled(GL_DEPTH_TEST);
			GLboolean blendingEnable = glIsEnabled(GL_BLEND);

			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);

			// set up text data
			vector2 halfScreeSize = gui::UIManager::instance().getScreenSize() * 0.5f;
			m_textRendering->setVector<UIUniforms>(UIUF::HALF_SCREEN_SIZE, vector4(halfScreeSize.x, halfScreeSize.y, 
																				   1.0f / halfScreeSize.x, -1.0f / halfScreeSize.y));
			m_textRendering->setVector<UIUniforms>(UIUF::TEXTURE_SIZE, vector2(1.0f / (float)fontResource->getTexture()->getWidth(),
																			   1.0f / (float)fontResource->getTexture()->getHeight()));				
			m_textRendering->setVector<UIUniforms>(UIUF::TEXT_COLOR, label->getColor());
			m_textRendering->setVector<UIUniforms>(UIUF::AREA, vector4(cachePtr->position.x, cachePtr->position.y,
																	   cachePtr->position.x + cachePtr->size.x, 
																	   cachePtr->position.y + cachePtr->size.y));
			fontResource->getTexture()->setToSampler(m_textRendering->getUniform<UIUniforms>(UIUF::CHARACTERS_MAP));
				
			auto charIt = cachePtr->characters.begin();
			size_t callsCount = (cachePtr->characters.size() / MAX_CHARACTERS_PER_CALL) + 1;
			for (size_t call = 0; call < callsCount; call++)
			{
				// set up characters data
				size_t i = 0;
				for (; i < MAX_CHARACTERS_PER_CALL; i++, ++charIt)
				{
					if (charIt == cachePtr->characters.end()) break;
					CharacterData charDat;
					charDat.rectangle = charIt->box;
					charDat.uv = charIt->texturePos;
					m_charactersDataBuffer->setElement(i, std::move(charDat));
				}
				m_textRendering->setUniformBuffer<UIUniforms>(UIUF::CHARACTERS_DATA, *m_charactersDataBuffer, 0);

				// draw
				glDrawArraysInstanced(GL_POINTS, 0, 1, i);
			}

			if (depthTestEnable) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
			if (blendingEnable) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		}
	}
}

}