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

#include "dds/DDSTextureLoader.h"
#include <FreeImage.h>

namespace framework
{

struct ImageInfo
{
	FIBITMAP* dib;
	BYTE* data;
	DWORD width;
	DWORD height;
	unsigned int bpp;
	ImageInfo() : dib(0), data(0), width(0), height(0), bpp(0) {}
};

namespace
{
	void freeImageOutput(FREE_IMAGE_FORMAT fif, const char *msg)
	{
		utils::Logger::toLogWithFormat("FreeImage: %s.\n", msg);
	}

	bool gatherImageInfo(ImageInfo& info, const std::string& fileName)
	{
		if (!utils::Utils::exists(fileName))
		{
			utils::Logger::toLogWithFormat("Error: file '%s' has not been found.\n", fileName.c_str());
			return false;
		}

		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(fileName.c_str(), 0);
		if (fif == FIF_UNKNOWN)
		{
			fif = FreeImage_GetFIFFromFilename(fileName.c_str());
		}
		if (fif == FIF_UNKNOWN)
		{
			utils::Logger::toLogWithFormat("Error: format of file '%s' is unknown.\n", fileName.c_str());
			return false;
		}

		info.dib = FreeImage_Load(fif, fileName.c_str());
		if (info.dib == 0)
		{
			utils::Logger::toLogWithFormat("Error: could not load file '%s'.\n", fileName.c_str());
			return false;
		}

		FreeImage_FlipVertical(info.dib);

		info.data = FreeImage_GetBits(info.dib);
		info.width = FreeImage_GetWidth(info.dib);
		info.height = FreeImage_GetHeight(info.dib);
		info.bpp = FreeImage_GetBPP(info.dib);

		return true;
	}
}

class DdsLoader : public Texture::Loader
{
public:
    virtual bool load(Texture* texture, const std::string& fileName)
    {
		if (fileName.empty())
		{
			utils::Logger::toLogWithFormat("Error: could not create a texture, file name is empty.\n", fileName.c_str());
			return false;
		}

		if (!utils::Utils::exists(fileName))
		{
			utils::Logger::toLogWithFormat("Error: file '%s' has not been found.\n", fileName.c_str());
			return false;
		}

		// path to unicode
		std::wstring unicodeFileName = utils::Utils::toUnicode(fileName);

		// load texture
		const Device& device = Application::instance()->getDevice();
		ID3D11Resource* resource = 0;
		HRESULT hr = DirectX::CreateDDSTextureFromFile(device.device, unicodeFileName.c_str(), (ID3D11Resource**)&resource, &texture->m_view);
		if (hr != S_OK)
		{
			utils::Logger::toLogWithFormat("Error: could not load a texture '%s'.\n", fileName.c_str());
			return false;
		}

		D3D11_RESOURCE_DIMENSION resdim;
		resource->GetType(&resdim);
		if (resdim == D3D11_RESOURCE_DIMENSION_TEXTURE1D)
		{
			texture->m_texture1D = (ID3D11Texture1D*)resource;
			texture->m_texture1D->GetDesc(&texture->m_texture1DDesc);
			texture->m_type = Texture1D;
		}
		else if (resdim == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		{
			texture->m_texture2D = (ID3D11Texture2D*)resource;
			texture->m_texture2D->GetDesc(&texture->m_texture2DDesc);
			texture->m_type = Texture2D;
		}
		else if (resdim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
		{
			texture->m_texture3D = (ID3D11Texture3D*)resource;
			texture->m_texture3D->GetDesc(&texture->m_texture3DDesc);
			texture->m_type = Texture3D;
		}
		else
		{
			resource->Release();
			texture->destroy();
			return false;
		}
        
        return true;
    }
};

Texture::Texture() :
	m_texture1D(0),
	m_texture2D(0),
	m_texture3D(0),
	m_type(TextureUnknown),
	m_view(0)
{
}

Texture::~Texture()
{
	destroy();
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

bool Texture::initWithDDS(const std::string& fileName)
{
    static DdsLoader loader;
	destroy();
	bool result = loader.load(this, fileName);

	if (result) initDestroyable();
	return result;
}

bool Texture::initWithData( DXGI_FORMAT format, const std::vector<unsigned char>& buffer, size_t width, size_t height )
{
	const Device& device = Application::instance()->getDevice();
	destroy();

	m_type = Texture2D;

	m_texture2DDesc.Width = width;
	m_texture2DDesc.Height = height;
	m_texture2DDesc.MipLevels = 1;
	m_texture2DDesc.ArraySize = 1;
	m_texture2DDesc.Format = format;
	m_texture2DDesc.SampleDesc.Count = 1;
	m_texture2DDesc.SampleDesc.Quality = 0;
	m_texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
	m_texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	m_texture2DDesc.CPUAccessFlags = 0;
	m_texture2DDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = buffer.data();
	data.SysMemPitch = UtilitiesD3D11::sizeOfFormat(format) * width;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device.device->CreateTexture2D(&m_texture2DDesc, &data, &m_texture2D);
	if (hr != S_OK)
	{
		destroy();
		utils::Logger::toLog("Error: could not initialize a texture with data.\n");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2DArray.MipLevels = 1;
	desc.Texture2DArray.ArraySize = 1;
	hr = device.device->CreateShaderResourceView(m_texture2D, &desc, &m_view);
	if (hr != S_OK)
	{
		destroy();
		utils::Logger::toLog("Error: could not create shader resource view for a texture.\n");
		return false;
	}

	initDestroyable();

	return true;
}

void Texture::destroy()
{
	if (m_view != 0)
	{
		m_view->Release();
		m_view = 0;
	}

	if (m_texture1D != 0)
	{
		m_texture1D->Release();
		m_texture1D = 0;
	}
	if (m_texture2D != 0)
	{
		m_texture2D->Release();
		m_texture2D = 0;
	}
	if (m_texture3D != 0)
	{
		m_texture3D->Release();
		m_texture3D = 0;
	}
}

bool Texture::isValid() const
{
	return m_type != TextureUnknown && (m_texture1D != 0 || m_texture2D != 0 || m_texture3D != 0) && m_view != 0;
}

ID3D11Resource* Texture::getResource() const
{
	if (m_texture1D != 0)
		return m_texture1D;

	if (m_texture2D != 0)
		return m_texture2D;

	if (m_texture3D != 0)
		return m_texture3D;

	return 0;
}

std::vector<unsigned char> LoadHeightmapData(const std::string& fileName, unsigned int& width, unsigned int& height)
{
	ImageInfo info;
	if (!gatherImageInfo(info, fileName))
	{
		utils::Logger::toLogWithFormat("Error: could not get image info from the file '%s'.\n", fileName.c_str());
		if (info.dib != 0) FreeImage_Unload(info.dib);
		return std::vector<unsigned char>();
	}

	size_t bufSize = info.width * info.height;
	std::vector<unsigned char> buffer;
	buffer.resize(bufSize);
	size_t bytesPP = info.bpp / 8;
	for (size_t i = 0; i < bufSize; i++)
	{
		buffer[i] = info.data[i * bytesPP];
	}

	if (info.dib != 0)
	{
		FreeImage_Unload(info.dib);
	}

	width = info.width;
	height = info.height;

	return buffer;
}

}
