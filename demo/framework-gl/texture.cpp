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

#include "stdafx.h"
#include "texture.h"
#include <ktx.h>
#include <FreeImage.h>

namespace framework
{

class KtxLoader : public Texture::Loader
{
public:
    virtual ~KtxLoader() {}
    
    virtual bool load(Texture* texture, const std::string& fileName)
    {
		if (!utils::Utils::exists(fileName))
		{
			utils::Logger::toLogWithFormat("Error: file '%s' has not been found.\n", fileName.c_str());
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
			utils::Logger::toLog(std::string("Error: failed to load a texture, ") + fileName);
            return false;
        }
        
        texture->m_texture = tex;
        texture->m_target = target;

        return true;
    }
};

namespace
{
	void freeImageOutput(FREE_IMAGE_FORMAT fif, const char *msg)
	{
		utils::Logger::toLogWithFormat("FreeImage: %s.\n", msg);
	}

	int getPixelFormat(int textureFormat)
	{
		switch (textureFormat)
		{
		case GL_R8:	return GL_RED;
		case GL_R8_SNORM: return GL_RED;
		case GL_R16: return GL_RED;
		case GL_R16_SNORM: return GL_RED;
		case GL_RG8: return GL_RG;
		case GL_RG8_SNORM: return GL_RG;
		case GL_RG16: return GL_RG;
		case GL_RG16_SNORM: return GL_RG;
		case GL_R3_G3_B2: return GL_RGB;
		case GL_RGB4: return GL_RGB;
		case GL_RGB5: return GL_RGB;
		case GL_RGB8: return GL_RGB;
		case GL_RGB8_SNORM:	return GL_RGB;
		case GL_RGB10: return GL_RGB;
		case GL_RGB12: return GL_RGB;
		case GL_RGB16_SNORM: return GL_RGB;
		case GL_RGBA2: return GL_RGB;
		case GL_RGBA4: return GL_RGB;
		case GL_RGB5_A1: return GL_RGBA;
		case GL_RGBA8: return GL_RGBA;
		case GL_RGBA8_SNORM: return GL_RGBA;
		case GL_RGB10_A2: return GL_RGBA;
		case GL_RGB10_A2UI:	return GL_RGBA;
		case GL_RGBA12:	return GL_RGBA;
		case GL_RGBA16:	return GL_RGBA;
		case GL_SRGB8: return GL_RGB;
		case GL_SRGB8_ALPHA8: return GL_RGBA;
		case GL_R16F: return GL_RED;
		case GL_RG16F: return GL_RG;
		case GL_RGB16F: return GL_RGB;
		case GL_RGBA16F: return GL_RGBA;
		case GL_R32F: return GL_RED;
		case GL_RG32F: return GL_RG;
		case GL_RGB32F: return GL_RGB;
		case GL_RGBA32F: return GL_RGBA;
		case GL_R11F_G11F_B10F:	return GL_RGB;
		case GL_RGB9_E5: return GL_RGB;
		case GL_R8I: return GL_RED;
		case GL_R8UI: return GL_RED;
		case GL_R16I: return GL_RED;
		case GL_R16UI: return GL_RED;
		case GL_R32I: return GL_RED;
		case GL_R32UI: return GL_RED;
		case GL_RG8I: return GL_RG;
		case GL_RG8UI: return GL_RG;
		case GL_RG16I: return GL_RG;
		case GL_RG16UI: return GL_RG;
		case GL_RG32I: return GL_RG;
		case GL_RG32UI: return GL_RG;
		case GL_RGB8I: return GL_RGB;
		case GL_RGB8UI: return GL_RGB;
		case GL_RGB16I: return GL_RGB;
		case GL_RGB16UI: return GL_RGB;
		case GL_RGB32I: return GL_RGB;
		case GL_RGB32UI: return GL_RGB;
		case GL_RGBA8I:	return GL_RGBA;
		case GL_RGBA8UI: return GL_RGBA;
		case GL_RGBA16I: return GL_RGBA;
		case GL_RGBA16UI: return GL_RGBA;
		case GL_RGBA32I: return GL_RGBA;
		case GL_RGBA32UI: return GL_RGBA;
		}

		return -1;
	}
}

int Texture::m_freeTextureSlot = 0;

Texture::Texture():
    m_isLoaded(false),
	m_texture(0),
	m_target(0),
	m_width(0),
	m_height(0)
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

	if (m_isLoaded)
	{
		glBindTexture(m_target, m_texture);
		glGetTexLevelParameteriv(m_target, 0, GL_TEXTURE_WIDTH, (GLint*)&m_width);
		glGetTexLevelParameteriv(m_target, 0, GL_TEXTURE_HEIGHT, (GLint*)&m_height);
		setSampling();
		generateMipmaps();
		glBindTexture(m_target, 0);

		initDestroyable();
	}
	return m_isLoaded;
}
bool Texture::initWithData(GLint format, const std::vector<unsigned char>& buffer, size_t width, size_t height, bool mipmaps)
{
	destroy();

	m_isLoaded = true;

	m_target = GL_TEXTURE_2D;
	m_width = width;
	m_height = height;
	glGenTextures(1, &m_texture);
	glBindTexture(m_target, m_texture);
	glTexStorage2D(m_target, 1, format, width, height);
	glTexSubImage2D(m_target, 0, 0, 0, width, height, getPixelFormat(format), GL_UNSIGNED_BYTE, buffer.data());
	setSampling();
	if (mipmaps) generateMipmaps();
	glBindTexture(m_target, 0);

	if (CHECK_GL_ERROR)
	{
		destroy();
		return false;
	}

	initDestroyable();
	return m_isLoaded;
}

void Texture::setSampling()
{
	if (m_target != 0 && m_texture != 0)
	{
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
}
void Texture::generateMipmaps()
{
	if (m_target != 0)
	{
		glGenerateMipmap(m_target);
	}
}

void Texture::setToSampler(int samplerIndex)
{
    if (!m_isLoaded) return;
	if (samplerIndex < 0) return;
	if (m_freeTextureSlot >= MAX_BOUND_TEXTURES) return;
	
	glUniform1i(samplerIndex, m_freeTextureSlot);
	glActiveTexture(GL_TEXTURE0 + m_freeTextureSlot);
	glBindTexture(m_target, m_texture);

	m_freeTextureSlot++;
}

void Texture::init()
{
	FreeImage_Initialise();
	FreeImage_SetOutputMessage(freeImageOutput);
}

void Texture::cleanup()
{
	FreeImage_DeInitialise();
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
	if (m_texture != 0)
	{
		glDeleteTextures(1, &m_texture);
	}
	m_isLoaded = false;
}

void saveTextureToPng(const std::string& filename, std::shared_ptr<Texture> texture)
{
	if (!texture) return;

	std::unique_ptr<unsigned char[]> pixels;
	FILE *image = 0;

	glBindTexture(texture->m_target, texture->m_texture);

	GLint sz = 0;	
	GLint componentSize = 0;
	glGetTexLevelParameteriv(texture->m_target, 0, GL_TEXTURE_RED_SIZE, &componentSize);
	sz += componentSize;
	glGetTexLevelParameteriv(texture->m_target, 0, GL_TEXTURE_GREEN_SIZE, &componentSize);
	sz += componentSize;
	glGetTexLevelParameteriv(texture->m_target, 0, GL_TEXTURE_BLUE_SIZE, &componentSize);
	sz += componentSize;
	glGetTexLevelParameteriv(texture->m_target, 0, GL_TEXTURE_ALPHA_SIZE, &componentSize);
	sz += componentSize;
	sz /= 8;

	GLint format = 0;
	glGetTexLevelParameteriv(texture->m_target, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

	size_t bufSize = texture->getWidth() * texture->getHeight() * sz;
	pixels.reset(new unsigned char[bufSize]);

	glGetTexImage(texture->m_target, 0, getPixelFormat(format), GL_UNSIGNED_BYTE, pixels.get());
	if (CHECK_GL_ERROR) return;
	
	FIBITMAP *bitmap = FreeImage_AllocateT(FIT_BITMAP, texture->getWidth(), texture->getHeight(), sz * 8);
	if (bitmap != 0)
	{
		auto dat = FreeImage_GetBits(bitmap);
		memcpy(dat, pixels.get(), bufSize);
		BOOL r = FreeImage_Save(FIF_PNG, bitmap, filename.c_str());
		FreeImage_Unload(bitmap);
	}
}

}
