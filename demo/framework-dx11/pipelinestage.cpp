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

#include "pipelinestage.h"
#include "application.h"
#include "utils.h"
#include <algorithm>

namespace framework
{

void PipelineStage::init(const Device& device)
{
	destroy();
	onInit(device);
	initDestroyable();
}

void PipelineStage::apply(const Device& device)
{
	onApply(device);
	Application::Instance()->getPipelineStageManager().pushPipelineStage(std::static_pointer_cast<PipelineStage>(shared_from_this()));
}

void PipelineStage::cancel(const Device& device)
{
	onCancel(device);
	Application::Instance()->getPipelineStageManager().popPipelineStage(std::static_pointer_cast<PipelineStage>(shared_from_this()), device);
}

PipelineStageManager::PipelineStageManager()
{
	const int STAGES_MAX_DEPTH = 10;
	for (size_t i = 0; i < PipelineStageCount; i++)
	{
		m_stages[i].resize(STAGES_MAX_DEPTH);
		m_indices[i] = 0;
	}
}

void PipelineStageManager::pushPipelineStage(std::shared_ptr<PipelineStage> stagePtr)
{
	auto type = stagePtr->GetType();
	int index = m_indices[type];
	if (index >= (int)m_stages[type].size())
	{
		m_stages[type].push_back(stagePtr);
	}
	else
	{
		m_stages[type][index] = stagePtr;
	}
	m_indices[type]++;
}

void PipelineStageManager::popPipelineStage(std::shared_ptr<PipelineStage> stagePtr, const Device& device)
{
	auto type = stagePtr->GetType();

	for (int i = m_indices[type] - 1; i >= 0; i--)
	{
		if (m_stages[type][i] == stagePtr)
		{
			bool needApply = (i == m_indices[type] - 1); // applied stage is the last always
			m_stages[type][i].reset();
			if (needApply)
			{
				int j = i - 1;
				for (; j >= 0; j--) // looking for previous stage
				{
					if (m_stages[type][j].get() != 0)
					{
						if (m_stages[type][j] != stagePtr) m_stages[type][j]->onApply(device);
						m_indices[type] = j;
						break;
					}
				}
				if (j <= 0) m_indices[type] = 0;
			}
			break;
		}
	}
}

void PipelineStageManager::beginFrame(const Device& device, std::shared_ptr<RenderTarget> renderTarget)
{
	clearRenderTarget(device, std::move(renderTarget));
}

void PipelineStageManager::endFrame()
{
	for (size_t i = 0; i < PipelineStageCount; i++)
	{
		for (auto s : m_stages[i]) s.reset();
		m_indices[i] = 0;
	}
}

void PipelineStageManager::clearRenderTarget(const Device& device, std::shared_ptr<RenderTarget> renderTarget, const vector4& color, float depth, unsigned int stencil)
{
	ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	ID3D11DepthStencilView* depthStencil = 0;
	renderTargets[0] = renderTarget->getView().asRenderTargetView();

	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		if (renderTargets[i] != 0)
		{
			device.context->ClearRenderTargetView(renderTargets[i], utils::Utils::convert(color));
		}
	}

	if (depthStencil != 0)
	{
		device.context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
}

void PipelineStageManager::setRenderTarget(const Device& device, std::shared_ptr<RenderTarget> renderTarget)
{
	ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	ID3D11DepthStencilView* depthStencil = 0;
	renderTargets[0] = renderTarget->getView().asRenderTargetView();

	device.context->OMSetRenderTargets(1, renderTargets, depthStencil);
}


}