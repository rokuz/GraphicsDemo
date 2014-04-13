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

#pragma once
#include "structs.h"
#include <vector>

#include "vector.h"
#include "destroyable.h"
#include "structs.h"
#include "renderTarget.h"

namespace framework
{

class PipelineStageManager;

enum PipelineStageType
{
	RasterizerStageType = 0,
	DepthStencilStageType,
	BlendStageType,
	PipelineStageCount
};

class PipelineStage : public Destroyable
{
	friend class Application;
	friend class PipelineStageManager;
	
public:
	virtual ~PipelineStage(){}
	virtual PipelineStageType GetType() const = 0;
	virtual bool isValid() { return true; }
	
	void init(const Device& device);
	void apply(const Device& device);
	void cancel(const Device& device);
	
protected:
	virtual void onInit(const Device& device){}
	virtual void onApply(const Device& device){}
	virtual void onCancel(const Device& device){}
};

class PipelineStageManager
{
	friend class PipelineStage;

public:
	PipelineStageManager();
	void beginFrame(const Device& device, std::shared_ptr<RenderTarget> renderTarget);
	void endFrame();

	void clearRenderTarget(const Device& device, std::shared_ptr<RenderTarget> renderTarget, const vector4& color = vector4(0, 0, 0, 0), float depth = 1.0f, unsigned int stencil = 0);
	void setRenderTarget(const Device& device, std::shared_ptr<RenderTarget> renderTarget);

private:
	void pushPipelineStage(std::shared_ptr<PipelineStage> stagePtr);
	void popPipelineStage(std::shared_ptr<PipelineStage> stagePtr, const Device& device);

	std::vector<std::shared_ptr<PipelineStage> > m_stages[PipelineStageCount];
	int m_indices[PipelineStageCount];
};

}