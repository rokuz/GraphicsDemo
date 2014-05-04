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
#undef min
#undef max

namespace framework
{

void PipelineStage::init()
{
	const Device& device = Application::instance()->getDevice();
	destroy();
	onInit(device);
	initDestroyable();
}

void PipelineStage::apply()
{
	const Device& device = Application::instance()->getDevice();
	onApply(device);
	Application::instance()->getPipeline().pushPipelineStage(std::static_pointer_cast<PipelineStage>(shared_from_this()));
}

void PipelineStage::cancel()
{
	const Device& device = Application::instance()->getDevice();
	onCancel(device);
	Application::instance()->getPipeline().popPipelineStage(std::static_pointer_cast<PipelineStage>(shared_from_this()), device);
}

Pipeline::Pipeline()
{
	const int STAGES_MAX_DEPTH = 10;
	for (size_t i = 0; i < PipelineStageCount; i++)
	{
		m_stages[i].resize(STAGES_MAX_DEPTH);
		m_indices[i] = 0;
	}
}

void Pipeline::pushPipelineStage(std::shared_ptr<PipelineStage> stagePtr)
{
	auto type = stagePtr->GetType();
	int index = m_indices[type];
	if (index > 0 && m_stages[type][index - 1] == stagePtr)
	{
		return;
	}
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

void Pipeline::popPipelineStage(std::shared_ptr<PipelineStage> stagePtr, const Device& device)
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
						m_indices[type] = j + 1;
						break;
					}
				}
				if (j < 0) m_indices[type] = 0;
			}
			break;
		}
	}
}

void Pipeline::beginFrame(std::shared_ptr<RenderTarget> renderTarget)
{
	clearRenderTarget(std::move(renderTarget));
}

void Pipeline::endFrame()
{
	for (size_t i = 0; i < PipelineStageCount; i++)
	{
		for (auto s : m_stages[i]) s.reset();
		m_indices[i] = 0;
	}

	const Device& device = Application::instance()->getDevice();
	device.context->ClearState();
}

void Pipeline::clearRenderTarget(std::shared_ptr<RenderTarget> renderTarget, const vector4& color, float depth, unsigned int stencil)
{
	const Device& device = Application::instance()->getDevice();

	ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	int cnt = std::min(renderTarget->getViewCount(), D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	for (int i = 0; i < cnt; i++)
	{
		renderTargets[i] = renderTarget->getView(i).asRenderTargetView();
	}
	ID3D11DepthStencilView* depthStencil = renderTarget->isDepthUsed() ? renderTarget->getDepthView().asDepthStencilView() : 0;
	
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

void Pipeline::clearUnorderedAccessBatch(const UnorderedAccessibleBatch& uabatch, unsigned int value)
{
	const Device& device = Application::instance()->getDevice();

	unsigned int clearValue[4] = { value, value, value, value };
	for (int i = 0; i < uabatch.elementsNumber; i++)
	{
		device.context->ClearUnorderedAccessViewUint(uabatch.elements[i], clearValue);
	}
}

void Pipeline::setRenderTarget(std::shared_ptr<RenderTarget> renderTarget)
{
	const Device& device = Application::instance()->getDevice();

	ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	int cnt = std::min(renderTarget->getViewCount(), D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	for (int i = 0; i < cnt; i++)
	{
		renderTargets[i] = renderTarget->getView(i).asRenderTargetView();
	}
	ID3D11DepthStencilView* depthStencil = renderTarget->isDepthUsed() ? renderTarget->getDepthView().asDepthStencilView() : 0;

	device.context->OMSetRenderTargets(cnt, renderTargets, depthStencil);
}

void Pipeline::setRenderTarget(std::shared_ptr<RenderTarget> renderTarget, std::shared_ptr<RenderTarget> depthStencil)
{
	const Device& device = Application::instance()->getDevice();

	ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	int cnt = std::min(renderTarget->getViewCount(), D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	for (int i = 0; i < cnt; i++)
	{
		renderTargets[i] = renderTarget->getView(i).asRenderTargetView();
	}

	ID3D11DepthStencilView* dsv = depthStencil->isDepthUsed() ? depthStencil->getDepthView().asDepthStencilView() : 0;

	device.context->OMSetRenderTargets(cnt, renderTargets, dsv);
}

void Pipeline::setRenderTarget(std::shared_ptr<RenderTarget> renderTarget, const UnorderedAccessibleBatch& uabatch)
{
	const Device& device = Application::instance()->getDevice();

	ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
	int cnt = std::min(renderTarget->getViewCount(), D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	for (int i = 0; i < cnt; i++)
	{
		renderTargets[i] = renderTarget->getView(i).asRenderTargetView();
	}
	ID3D11DepthStencilView* depthStencil = renderTarget->isDepthUsed() ? renderTarget->getDepthView().asDepthStencilView() : 0;

	if (cnt + uabatch.elementsNumber > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)
	{
		utils::Logger::toLog("Error: setting render target failed, simultaneous render target views count exceeded.\n");
		return;
	}

	device.context->OMSetRenderTargetsAndUnorderedAccessViews(cnt, renderTargets, depthStencil, 
															  cnt, uabatch.elementsNumber, 
															  uabatch.elements, uabatch.initialValues);
}

void Pipeline::drawPoints( unsigned int count )
{
	const Device& device = Application::instance()->getDevice();
	device.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	device.context->Draw(count, 0);
}


}