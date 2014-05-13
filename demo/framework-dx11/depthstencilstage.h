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

namespace framework
{

class DepthStencilStage : public PipelineStage
{
public:
	DepthStencilStage();
	virtual ~DepthStencilStage();
	virtual PipelineStageType GetType() const { return DepthStencilStageType; }
	virtual bool isValid();
	virtual void destroy();

	void initWithDescription(const D3D11_DEPTH_STENCIL_DESC& desc);
	
	void setStencilRef(unsigned int stencilRef) { m_stencilRef = stencilRef; }
	unsigned int getStencilRef() const { return m_stencilRef; }

	static const D3D11_DEPTH_STENCIL_DESC& getDefault();
	static D3D11_DEPTH_STENCIL_DESC getDisableDepthWriting();

protected:
	virtual void onInit(const Device& device);
	virtual void onApply(const Device& device);

private:
	D3D11_DEPTH_STENCIL_DESC m_description;
	ID3D11DepthStencilState* m_state;
	unsigned int m_stencilRef;
};

}