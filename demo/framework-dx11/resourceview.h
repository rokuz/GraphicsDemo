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

class ResourceView
{
	friend class RenderTarget;
	friend class UniformBuffer;
	friend class UnorderedAccessBuffer;

	static D3D11_SHADER_RESOURCE_VIEW_DESC getDefaultShaderDesc();
	static D3D11_RENDER_TARGET_VIEW_DESC getDefaultRenderTargetDesc();
	static D3D11_DEPTH_STENCIL_VIEW_DESC getDefaultDepthStencilDesc();
	static D3D11_UNORDERED_ACCESS_VIEW_DESC getDefaultUAVDesc();

	void setShaderDesc(const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
	void setRenderTargetDesc(const D3D11_RENDER_TARGET_VIEW_DESC& desc);
	void setDepthStencilDesc(const D3D11_DEPTH_STENCIL_VIEW_DESC& desc);
	void setUnorderedAccessDesc(const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
	void init(const Device& device, ID3D11Resource* resource, unsigned int bindFlags);
	void destroy();
	bool isValid() const;

public:
	ResourceView();
	~ResourceView();

	ID3D11ShaderResourceView* asShaderView() const { return m_shaderView; }
	ID3D11RenderTargetView* asRenderTargetView() const { return m_renderTargetView; }
	ID3D11DepthStencilView* asDepthStencilView() const { return m_depthStencilView; }
	ID3D11UnorderedAccessView* asUAView() const { return m_uaView; }

private:
	std::pair<D3D11_SHADER_RESOURCE_VIEW_DESC, bool> m_shaderDesc;
	ID3D11ShaderResourceView* m_shaderView;
	std::pair<D3D11_RENDER_TARGET_VIEW_DESC, bool> m_renderTargetDesc;
	ID3D11RenderTargetView* m_renderTargetView;
	std::pair<D3D11_DEPTH_STENCIL_VIEW_DESC, bool> m_depthStencilDesc;
	ID3D11DepthStencilView* m_depthStencilView;
	std::pair<D3D11_UNORDERED_ACCESS_VIEW_DESC, bool> m_uavDesc;
	ID3D11UnorderedAccessView* m_uaView;
};

}