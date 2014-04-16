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
#include <string>
#include <vector>

namespace framework
{

enum TextureType
{
	TextureUnknown = 0,
	Texture1D,
	Texture2D,
	Texture3D
};

class Texture : public Destroyable
{
    friend class DdsLoader;
	friend class Application;
    
	TextureType m_type;
	ID3D11Texture1D* m_texture1D;
	ID3D11Texture2D* m_texture2D;
	ID3D11Texture3D* m_texture3D;
	D3D11_TEXTURE1D_DESC m_texture1DDesc;
	D3D11_TEXTURE2D_DESC m_texture2DDesc;
	D3D11_TEXTURE3D_DESC m_texture3DDesc;
	ID3D11ShaderResourceView* m_view;

	virtual void destroy();

public:
    Texture();
	virtual ~Texture();
    
    bool initWithDDS(const std::string& fileName);
	bool isValid() const;

	TextureType getType() const { return m_type; }
	ID3D11Texture1D* getTexture1D() const { return m_texture1D; }
	ID3D11Texture2D* getTexture2D() const { return m_texture2D; }
	ID3D11Texture3D* getTexture3D() const { return m_texture3D; }
	const D3D11_TEXTURE1D_DESC& getDesc1D() const { return m_texture1DDesc; }
	const D3D11_TEXTURE2D_DESC& getDesc2D() const { return m_texture2DDesc; }
	const D3D11_TEXTURE3D_DESC& getDesc3D() const { return m_texture3DDesc; }
	ID3D11ShaderResourceView* getView() const { return m_view; }

    class Loader
    {
    public:
        virtual ~Loader() {}
        virtual bool load(Texture* texture, const std::string& fileName) = 0;
    };
};

}