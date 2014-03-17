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

#include "renderingSettings.h"
#include "CEGUI/CEGUI.h"
#include <map>
#include <string>
#include <assert.h>

namespace framework
{

static std::map<int, std::string> MULTISAMPLING_TEXTS;

#define MS_TEXT(level, text) if (MULTISAMPLING_TEXTS.find(level) != MULTISAMPLING_TEXTS.end()) { text = MULTISAMPLING_TEXTS[level]; } else { text = MULTISAMPLING_TEXTS[-1]; }

int indexOf(int val, const std::vector<int>& levels)
{
	for (size_t i = 0; i < levels.size(); i++)
	{
		if (levels[i] == val) return (int)i;
	}
	return -1;
}

RenderingSettings::RenderingSettings() :
	m_handler(0),
	m_multisamplingCombo(0)
{
	static bool isStaticInit = false;
	if (!isStaticInit)
	{
		MULTISAMPLING_TEXTS.insert(std::make_pair(0, "none"));
		MULTISAMPLING_TEXTS.insert(std::make_pair(1, "1x"));
		MULTISAMPLING_TEXTS.insert(std::make_pair(2, "2x"));
		MULTISAMPLING_TEXTS.insert(std::make_pair(4, "4x"));
		MULTISAMPLING_TEXTS.insert(std::make_pair(8, "8x"));
		MULTISAMPLING_TEXTS.insert(std::make_pair(16, "16x"));
		MULTISAMPLING_TEXTS.insert(std::make_pair(-1, "undefined"));
		isStaticInit = true;
	}
}

RenderingSettings::~RenderingSettings()
{

}

void RenderingSettings::init(CEGUI::DefaultWindow* window, 
							 const std::vector<int>& multisamplingLevels,
							 int multisamplingLevel)
{
	m_multisamplingLevels = multisamplingLevels;

	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

	CEGUI::FrameWindow* wnd = (CEGUI::FrameWindow*)winMgr.loadLayoutFromFile("RenderingSettings.wnd");
	window->addChild(wnd);

	m_multisamplingCombo = (CEGUI::Combobox*)wnd->getChildElementRecursive("MultisamplingLevelCombobox");
	if (m_multisamplingCombo != 0)
	{
		for (size_t i = 0; i < multisamplingLevels.size(); i++)
		{
			std::string name;
			MS_TEXT(multisamplingLevels[i], name);
			m_multisamplingCombo->addItem(new CEGUI::ListboxTextItem(name.c_str(), i));
		}
		int index = indexOf(multisamplingLevel, m_multisamplingLevels);
		assert(index >= 0);
		m_multisamplingCombo->setItemSelectState((size_t)index, true);
		m_multisamplingCombo->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&RenderingSettings::handleMultisamplingChanging, this));
	}
}

void RenderingSettings::setHandler( IRenderingSettingsHandler* handler )
{
	m_handler = handler;
}

bool RenderingSettings::handleMultisamplingChanging( const CEGUI::EventArgs& e )
{
	CEGUI::ListboxItem* item = m_multisamplingCombo->getSelectedItem();
	if (item != 0)
	{
		int newLevel = m_multisamplingLevels[item->getID()];
		if (m_handler != 0) { m_handler->onChangeMultisamplingLevel(newLevel); }
	}

	return true;
}

}