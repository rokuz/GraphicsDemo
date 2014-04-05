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
#include "pipelinestage.h"

namespace framework
{

class RasterizerStage : public PipelineStage
{
public:
	RasterizerStage();
	virtual ~RasterizerStage();
	virtual PipelineStageType GetType() const { return RasterizerStageType; }
	virtual bool isValid();
	virtual void destroy();

	void initWithDescription(const Device& device, const D3D11_RASTERIZER_DESC& desc);
	
	void addViewport(const D3D11_VIEWPORT& viewport);
	void clearViewports();
	int getViewportsCount() const;
	void addScissorRect(const D3D11_RECT& rect);
	void clearScissorRects();
	int getScissorRectsCount() const;

	static const D3D11_RASTERIZER_DESC& getDefault();
	static D3D11_VIEWPORT getDefaultViewport(int width, int height);
	static D3D11_RECT getDefaultScissorRect(int x, int y, int width, int height);

protected:
	virtual void onInit(const Device& device);
	virtual void onApply(const Device& device);

private:
	D3D11_RASTERIZER_DESC m_description;
	ID3D11RasterizerState* m_state;
	std::vector<D3D11_VIEWPORT> m_viewports;
	std::vector<D3D11_RECT> m_scissorRects;
};

}