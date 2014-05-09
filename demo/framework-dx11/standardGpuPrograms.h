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

#include <memory>
#include "gpuprogram.h"

namespace framework
{

DECLARE_UNIFORMS_BEGIN(StandardUniforms)
	LINE_RENDERER_DATA,
	ARROW_RENDERER_DATA,
DECLARE_UNIFORMS_END()

#pragma pack (push, 1)
struct LineRendererData
{
	matrix44 modelViewProjection;
	vector4 color;
};
#pragma pack (pop)

#pragma pack (push, 1)
struct ArrowRendererData
{
	matrix44 modelViewProjection;
	vector3 position;
	unsigned int : 32;
	quaternion orientation;
	vector4 color;
};
#pragma pack (pop)

const std::string STANDARD_SHADERS_PATH = "data/shaders/dx11/standard/";

class StandardGpuPrograms
{
public:

	static bool init();

	static std::shared_ptr<GpuProgram> getLineRenderer();
	static std::shared_ptr<GpuProgram> getArrowRenderer();

private:
	static std::shared_ptr<GpuProgram> m_lineRenderer;
	static std::shared_ptr<GpuProgram> m_arrowRenderer;
};

}

#define STD_UF framework::UniformBase<framework::StandardUniforms>::Uniform