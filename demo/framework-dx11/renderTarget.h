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
#include "destroyable.h"
#include "resourceview.h"
#include <vector>

namespace framework
{

class RenderTarget : public Destroyable
{
	friend class Application;

public:
	RenderTarget();
	virtual ~RenderTarget();

	static D3D11_TEXTURE2D_DESC getDefaultDesc(int width, int height, DXGI_FORMAT format);
	static D3D11_TEXTURE2D_DESC getDefaultDepthDesc(int width, int height);

	void initWithSwapChain(const Device& device);
	void initWithDescription(const D3D11_TEXTURE2D_DESC& desc, bool withDepth);
	void initWithDescriptions(const std::vector<D3D11_TEXTURE2D_DESC>& descs, bool withDepth);
	
	bool isValid() const;
	const ResourceView& getView(int index) const;
	int getViewCount() const;

	bool isDepthUsed() const;
	const ResourceView& getDepthView() const;

	const D3D11_TEXTURE2D_DESC& getDesc(int index) const;

private:
	virtual void destroy();
	bool createDepth(size_t index);
	bool createView(size_t index);

	bool m_isSwapChain;
	std::vector<ID3D11Texture2D*> m_colorBuffer;
	std::vector<D3D11_TEXTURE2D_DESC> m_colorBufferDesc;
	std::vector<ResourceView> m_view;
	bool m_useDepth;
	ID3D11Texture2D* m_depthBuffer;
	D3D11_TEXTURE2D_DESC m_depthBufferDesc;
	ResourceView m_depthView;
};

}