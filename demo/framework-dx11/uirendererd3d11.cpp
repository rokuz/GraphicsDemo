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
#include "uirendererd3d11.h"

namespace framework
{

const std::string GUI_SHADERS_PATH = "data/gui/shaders/dx11/";
const size_t MAX_CHARACTERS_PER_CALL = 256;

DECLARE_UNIFORMS_BEGIN(UIUniforms)
	CHARACTERS_DATA,
	TEXT_DATA,
	CHARACTERS_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UIUF UniformBase<UIUniforms>::Uniform

#pragma pack (push, 1)
struct TextData
{
	vector4 textColor;
	vector4 area;
	vector4 halfScreenSize;
	vector2 textureSize;
	unsigned int : 32;
	unsigned int : 32;
};
#pragma pack (pop)

#pragma pack (push, 1)
struct CharacterData
{
	vector4 rectangle;
	vector2 uv;
};
#pragma pack (pop)

bool UIRendererD3D11::init()
{
	// text rendering shader
	m_textRendering.reset(new framework::GpuProgram());
	m_textRendering->addShader(GUI_SHADERS_PATH + "text.vsh.hlsl");
	m_textRendering->addShader(GUI_SHADERS_PATH + "text.gsh.hlsl");
	m_textRendering->addShader(GUI_SHADERS_PATH + "text.psh.hlsl");
	if (!m_textRendering->init(true))
	{
		utils::Logger::toLog("Error: could not initialize text rendering shader.\n");
		return false;
	}
	m_textRendering->bindUniform<UIUniforms>(UIUF::CHARACTERS_DATA, "charactersData");
	m_textRendering->bindUniform<UIUniforms>(UIUF::TEXT_DATA, "textData");
	m_textRendering->bindUniform<UIUniforms>(UIUF::CHARACTERS_MAP, "charactersMap");
	m_textRendering->bindUniform<UIUniforms>(UIUF::DEFAULT_SAMPLER, "defaultSampler");

	// text data buffer
	m_textDataBuffer.reset(new framework::UniformBuffer());
	if (!m_textDataBuffer->initDefaultConstant<TextData>())
	{
		utils::Logger::toLog("Error: could not initialize text data buffer.\n");
		return false;
	}

	// characters data buffer
	m_charactersDataBuffer.reset(new framework::UniformBuffer());
	if (!m_charactersDataBuffer->initDefaultStructured<CharacterData>(MAX_CHARACTERS_PER_CALL))
	{
		utils::Logger::toLog("Error: could not initialize characters data buffer.\n");
		return false;
	}

	return true;
}

void UIRendererD3D11::cleanup()
{
	m_charactersDataBuffer.reset();
	m_textDataBuffer.reset();
	m_textRendering.reset();
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
		// TODO: implement
	}
	
	std::for_each(widget->getChildren().cbegin(), widget->getChildren().cend(), [&](gui::WidgetWeakPtr_T ptr)
	{
		if (!ptr.expired())
			renderWidget(std::move(ptr.lock()));
	});
}

void UIRendererD3D11::renderLabel(gui::LabelPtr_T label)
{
	const Device& device = Application::instance()->getDevice();

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
		if (cachePtr->characters.empty()) return;

		vector2 halfScreeSize = gui::UIManager::instance().getScreenSize() * 0.5f;

		// set up text data
		TextData textData;
		textData.textColor = label->getColor();
		textData.halfScreenSize = vector4(halfScreeSize.x, halfScreeSize.y, 1.0f / halfScreeSize.x, -1.0f / halfScreeSize.y);
		textData.area = vector4(cachePtr->position.x, cachePtr->position.y,
								cachePtr->position.x + cachePtr->size.x, cachePtr->position.y + cachePtr->size.y);
		textData.textureSize = vector2(1.0f / (float)fontResource->getTexture()->getDesc2D().Width,
									   1.0f / (float)fontResource->getTexture()->getDesc2D().Height);
		m_textDataBuffer->setData(textData);
		m_textDataBuffer->applyChanges();

		// rendering
		if (m_textRendering->use())
		{
			Application::instance()->disableDepthTest()->apply();
			Application::instance()->defaultAlphaBlending()->apply();

			m_textRendering->setUniform<UIUniforms>(UIUF::TEXT_DATA, m_textDataBuffer);
			m_textRendering->setUniform<UIUniforms>(UIUF::CHARACTERS_MAP, fontResource->getTexture());
			m_textRendering->setUniform<UIUniforms>(UIUF::DEFAULT_SAMPLER, Application::instance()->anisotropicSampler());

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
				m_charactersDataBuffer->applyChanges();

				// draw
				m_textRendering->setUniform<UIUniforms>(UIUF::CHARACTERS_DATA, m_charactersDataBuffer);
				device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
				device.context->DrawInstanced(1, i, 0, 0);
			}

			Application::instance()->disableDepthTest()->cancel();
			Application::instance()->defaultAlphaBlending()->cancel();
		}
	}
}

}