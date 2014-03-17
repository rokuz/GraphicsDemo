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

#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#ifdef WIN32
    #pragma once
#endif

#include "GL/gl3w.h"
#include "destroyable.h"
#include <string>
#include <vector>

namespace framework
{

#define MAX_BOUND_TEXTURES 32

class Texture : public Destroyable
{
    friend class KtxLoader;
	friend class Application;
    
    GLuint m_texture;
    GLenum m_target;
    bool m_isLoaded;
	static int m_freeTextureSlot;

	virtual void destroy();

public:
    Texture();
	virtual ~Texture();
    
    bool initWithKtx(const std::string& fileName);

	static void beginFrame();
	static void endFrame();
	static void resetSlots();
    
    void setToSampler(int samplerIndex);
    
    class Loader
    {
    public:
        virtual ~Loader() {}
        virtual bool load(Texture* texture, const std::string& fileName) = 0;
    };
};

}

#endif //__TEXTURE_H__