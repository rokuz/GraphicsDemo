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

#include "texture.h"
#include "logger.h"
#include "utils.h"
#include <ktx.h>

namespace framework
{

class KtxLoader : public Texture::Loader
{
public:
    KtxLoader()
    {
        
    }
    
    virtual ~KtxLoader()
    {
        
    }
    
    virtual bool load(Texture* texture, const std::string& fileName)
    {
		if (!utils::Utils::exists(fileName))
		{
			utils::Logger::toLogWithFormat("Fbx importer error: File '%s' has not been found.\n", fileName.c_str());
			return false;
		}

        KTX_error_code result;
        
        GLuint tex = 0;
        GLenum target = 0;
        KTX_dimensions dim;
        GLboolean hasMips = 0;
        GLenum error = 0;
        result = ktxLoadTextureN(fileName.c_str(), &tex, &target, &dim, &hasMips, &error, NULL, NULL);
        if (result != KTX_SUCCESS)
        {
			utils::Logger::toLog(std::string("Texture loading failed: ") + fileName);
            return false;
        }
        
        texture->m_texture = tex;
        texture->m_target = target;
        
        glBindTexture(GL_TEXTURE_2D, texture->m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        
        glGenerateMipmap(GL_TEXTURE_2D);
        
        return true;
    }
};

int Texture::m_freeTextureSlot = 0;

Texture::Texture():
    m_isLoaded(false)
{
    
}

Texture::~Texture()
{
	destroy();
}

bool Texture::initWithKtx(const std::string& fileName)
{
    static KtxLoader loader;
	destroy();
	m_isLoaded = loader.load(this, fileName);

	if (m_isLoaded) initDestroyable();
	return m_isLoaded;
}

void Texture::setToSampler(int samplerIndex)
{
    if (!m_isLoaded) return;
	if (samplerIndex < 0) return;
	if (m_freeTextureSlot >= MAX_BOUND_TEXTURES) return;
	
	glUniform1i(samplerIndex, m_freeTextureSlot);
	glActiveTexture(GL_TEXTURE0 + m_freeTextureSlot);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	m_freeTextureSlot++;
}

void Texture::resetSlots()
{
	m_freeTextureSlot = 0;
}

void Texture::beginFrame()
{
	m_freeTextureSlot = 0;
}

void Texture::endFrame()
{
	resetSlots();
}

void Texture::destroy()
{
	if (m_isLoaded)
	{
		glDeleteTextures(1, &m_texture);
		m_isLoaded = false;
	}
}

}
