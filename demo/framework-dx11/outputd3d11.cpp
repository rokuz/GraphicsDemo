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

#include "outputd3d11.h"
#include <sstream>

namespace framework
{

std::string toString(D3D_FEATURE_LEVEL featureLevel)
{
	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1:
		return "Direct 3D 9.1";
	case D3D_FEATURE_LEVEL_9_2:
		return "Direct 3D 9.2";
	case D3D_FEATURE_LEVEL_9_3:
		return "Direct 3D 9.3";
	case D3D_FEATURE_LEVEL_10_0:
		return "Direct 3D 10";
	case D3D_FEATURE_LEVEL_10_1:
		return "Direct 3D 10.1";
	case D3D_FEATURE_LEVEL_11_0:
		return "Direct 3D 11";
	default:
		return "Direct 3D Unknown";
	}
}

std::string toString(D3D11_FILL_MODE fillMode)
{
	switch (fillMode)
	{
	case D3D11_FILL_SOLID:
		return "Solid";
	case D3D11_FILL_WIREFRAME:
		return "Wireframe";
	default:
		return "Unknown";
	}
}

std::string toString(D3D11_CULL_MODE cullMode)
{
	switch (cullMode)
	{
	case D3D11_CULL_NONE:
		return "None";
	case D3D11_CULL_FRONT:
		return "Front";
	case D3D11_CULL_BACK:
		return "Back";
	default:
		return "Unknown";
	}
}

std::string toString(const D3D11_RASTERIZER_DESC& desc)
{
	std::stringstream ss;
	ss << "{ fill mode = " << toString(desc.FillMode)
	   << "; cull mode = " << toString(desc.CullMode)
	   << "; poligon order = " << (desc.FrontCounterClockwise ? "CCW" : "CW")
	   << "; depth bias = " << desc.DepthBias
	   << "; slope scaled depth bias = " << desc.SlopeScaledDepthBias
	   << "; depth bias clamp = " << desc.DepthBiasClamp
	   << "; depth clip enable = " << (desc.DepthClipEnable ? "Yes" : "No")
	   << "; scissor enable = " << (desc.ScissorEnable ? "Yes" : "No")
	   << "; multisample enable = " << (desc.MultisampleEnable ? "Yes" : "No")
	   << "; antialiased line enable = " << (desc.AntialiasedLineEnable ? "Yes" : "No")
	   << " }";
	return ss.str();
}

std::string toString(const D3D11_DEPTH_STENCIL_DESC& desc)
{
	// TODO: implement
	return "{ undefined }";
}

std::string toString(const D3D11_BLEND_DESC& desc)
{
	// TODO: implement
	return "{ undefined }";
}

std::string toString(const D3D11_SAMPLER_DESC& desc)
{
	// TODO: implement
	return "{ undefined }";
}

std::string toString(ShaderType shaderType)
{
	switch (shaderType)
	{
		case VERTEX_SHADER: return "vertex shader";
		case HULL_SHADER: return "hull shader";
		case DOMAIN_SHADER: return "domain shader";
		case GEOMETRY_SHADER: return "geometry shader";
		case PIXEL_SHADER: return "pixel shader";
		case COMPUTE_SHADER: return "compute shader";
	}
	return "";
}

}